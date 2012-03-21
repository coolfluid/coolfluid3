// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/static_assert.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/StreamHelpers.hpp"
#include "common/StringConversion.hpp"
#include "common/OptionArray.hpp"
#include "common/CreateComponentDataType.hpp"
#include "common/PropertyList.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

#include "mesh/actions/GlobalConnectivity.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/FaceCellConnectivity.hpp"
#include "mesh/NodeElementConnectivity.hpp"
#include "mesh/Node2FaceCellConnectivity.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "math/Functions.hpp"
#include "math/Consts.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/NodeElementConnectivity.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;
  using namespace math::Functions;
  using namespace math::Consts;

  create_component_data_type( std::vector<std::size_t> , mesh_actions_API , CVector_size_t , "CVector<size_t>" );

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < GlobalConnectivity, MeshTransformer, mesh::actions::LibActions> GlobalConnectivity_Builder;

//////////////////////////////////////////////////////////////////////////////

GlobalConnectivity::GlobalConnectivity( const std::string& name )
: MeshTransformer(name)
{

  properties()["brief"] = std::string("Construct global node and element numbering based on coordinates hash values");
  std::string desc;
  desc =
    "  Usage: GlobalConnectivity Regions:array[uri]=region1,region2\n\n";
  properties()["description"] = desc;
}

/// common tear-down for each test case
GlobalConnectivity::~GlobalConnectivity()
{
}


/////////////////////////////////////////////////////////////////////////////

std::string GlobalConnectivity::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string GlobalConnectivity::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" + properties().value<std::string>("description");
}

/////////////////////////////////////////////////////////////////////////////

void GlobalConnectivity::execute()
{
  Mesh& mesh = *m_mesh;

  Dictionary& nodes = mesh.geometry_fields();
  common::List<Uint>& nodes_glb_idx = nodes.glb_idx();
  // Undefined behavior if sizeof(Uint) != sizeof(std::size_t)
  // Assert at compile time
  //BOOST_STATIC_ASSERT(sizeof(std::size_t) == sizeof(Uint));

  // 1) make std::map<glb_node_idx,loc_node_idx>
  // 2) Make node2elem connectivity (does not contain elements from other partitions)
  // 3) foreach ghostnode, store connected owned elements
  // 4) broadcast (3) to all, and store the map node to ghost elements
  // 5) create the node to glb_elem_connectivity, as the combination of (2) and (4)



  //1)
  std::map<Uint,Uint> node_glb2loc;
  Uint loc_node_idx(0);
  boost_foreach(Uint glb_node_idx, nodes_glb_idx.array())
    node_glb2loc[glb_node_idx]=loc_node_idx++;

  //2)
  Handle<Component> node2elem_handle = mesh.geometry_fields().get_child("node2elem");
  if (node2elem_handle)
    mesh.geometry_fields().remove_component("node2elem");


  NodeElementConnectivity& node2elem = *mesh.geometry_fields().create_component<NodeElementConnectivity>("node2elem");
  node2elem.setup(mesh.topology());


  // 3)
  Uint nb_ghost(0);
  for (Uint i=0; i<nodes.size(); ++i)
    if (nodes.is_ghost(i))
      ++nb_ghost;

  std::vector<Uint> ghostnode_glb_idx(nb_ghost);
  std::vector<Uint> ghostnode_glb_elem_connectivity;
  std::vector<Uint> ghostnode_glb_elem_connectivity_start(nb_ghost+1);
  ghostnode_glb_elem_connectivity_start[0]=0;
  Handle< Component > elem_comp;
  Uint elem_idx;

  Uint cnt(0);
  for (Uint i=0; i<mesh.geometry_fields().size(); ++i)
  {
    if (mesh.geometry_fields().is_ghost(i))
    {
      ghostnode_glb_idx[cnt] = nodes_glb_idx[i];

      DynTable<Uint>::ConstRow elems = node2elem.connectivity()[i];
      boost_foreach(const Uint e, elems)
      {
        boost::tie(elem_comp,elem_idx) = node2elem.elements().location(e);
        ghostnode_glb_elem_connectivity.push_back(dynamic_cast<Elements&>(*elem_comp).glb_idx()[elem_idx]);
      }
      ghostnode_glb_elem_connectivity_start[cnt+1] = ghostnode_glb_elem_connectivity_start[cnt] + elems.size();

      ++cnt;
    }
  }

  // 4)
  std::vector<std::vector<Uint> > glb_elem_connectivity(nodes.size());
  nodes_glb_idx.resize(mesh.geometry_fields().size());

  for (Uint root=0; root<PE::Comm::instance().size(); ++root)
  {
    std::vector<Uint> rcv_glb_node_idx(0);//ghostnode_glb_idx.size());
    PE::Comm::instance().broadcast(ghostnode_glb_idx,rcv_glb_node_idx,root);
    std::vector<Uint> rcv_glb_elem_connectivity(0);//ghostnode_glb_elem_connectivity.size());
    PE::Comm::instance().broadcast(ghostnode_glb_elem_connectivity,rcv_glb_elem_connectivity,root);
    std::vector<Uint> rcv_glb_elem_connectivity_start(0);//ghostnode_glb_elem_connectivity_start.size());
    PE::Comm::instance().broadcast(ghostnode_glb_elem_connectivity_start,rcv_glb_elem_connectivity_start,root);

    if (PE::Comm::instance().rank() != root)
    {
      for (Uint p=0; p<PE::Comm::instance().size(); ++p)
      {
        if (p == PE::Comm::instance().rank())
        {
          Uint rcv_idx(0);
          boost_foreach(const std::size_t glb_node, rcv_glb_node_idx)
          {
            if (node_glb2loc.find(glb_node) != node_glb2loc.end())
            {
              //std::cout << "["<<PE::Comm::instance().rank() << "] owns ghostnode " << glb_node << " of [" << root << "]" << std::endl;
              Uint loc_node_idx = node_glb2loc[glb_node];
              for(Uint l=rcv_glb_elem_connectivity_start[rcv_idx]; l<rcv_glb_elem_connectivity_start[rcv_idx+1]; ++l)
                glb_elem_connectivity[loc_node_idx].push_back(rcv_glb_elem_connectivity[l]);
            }
            ++rcv_idx;
          }
          break;
        }
      }
    }
  }


  DynTable<Uint>& nodes_glb_elem_connectivity = mesh.geometry_fields().glb_elem_connectivity();
//  CFinfo << "nodes_glb_elem_connectivity = " << nodes_glb_elem_connectivity.uri() << CFendl;
  nodes_glb_elem_connectivity.resize(glb_elem_connectivity.size());
  for (Uint i=0; i<glb_elem_connectivity.size(); ++i)
  {
//    CFinfo << "i = " << i << CFendl;
    cf3_assert(i<node2elem.connectivity().size());
    DynTable<Uint>::ConstRow elems = node2elem.connectivity()[i];
    cf3_assert(i<nodes_glb_elem_connectivity.size());
    cf3_assert(i<glb_elem_connectivity.size());
    nodes_glb_elem_connectivity[i].resize(glb_elem_connectivity[i].size() + elems.size());
    cnt = 0;
    boost_foreach(const Uint e, elems)
    {
      cf3_assert(e<node2elem.elements().size());
      boost::tie(elem_comp,elem_idx) = node2elem.elements().location(e);
      cf3_assert(elem_idx < Handle<Elements>(elem_comp)->glb_idx().size());
      nodes_glb_elem_connectivity[i][cnt++] = Handle<Elements>(elem_comp)->glb_idx()[elem_idx];
    }
    for (Uint j=0; j<glb_elem_connectivity[i].size(); ++j)
    {
      cf3_assert(cnt < nodes_glb_elem_connectivity[i].size());
      nodes_glb_elem_connectivity[i][cnt++] = glb_elem_connectivity[i][j];
    }

  }

}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
