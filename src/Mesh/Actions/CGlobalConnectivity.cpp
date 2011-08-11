// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/static_assert.hpp>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"

#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/StreamHelpers.hpp"
#include "Common/StringConversion.hpp"
#include "Common/OptionArray.hpp"
#include "Common/CreateComponentDataType.hpp"

#include "Common/MPI/PE.hpp"
#include "Common/MPI/debug.hpp"

#include "Mesh/Actions/CGlobalConnectivity.hpp"
#include "Mesh/CCellFaces.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"
#include "Mesh/CNodeFaceCellConnectivity.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CMesh.hpp"
#include "Math/Functions.hpp"
#include "Math/Consts.hpp"
#include "Mesh/ElementData.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

  using namespace Common;
  using namespace Math::Functions;
  using namespace Math::Consts;

  create_component_data_type( std::vector<std::size_t> , Mesh_Actions_API , CVector_size_t , "CVector<size_t>" );

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CGlobalConnectivity, CMeshTransformer, LibActions> CGlobalConnectivity_Builder;

//////////////////////////////////////////////////////////////////////////////

CGlobalConnectivity::CGlobalConnectivity( const std::string& name )
: CMeshTransformer(name)
{

  properties()["brief"] = std::string("Construct global node and element numbering based on coordinates hash values");
  std::string desc;
  desc =
    "  Usage: CGlobalConnectivity Regions:array[uri]=region1,region2\n\n";
  properties()["description"] = desc;
}

/// common tear-down for each test case
CGlobalConnectivity::~CGlobalConnectivity()
{
}


/////////////////////////////////////////////////////////////////////////////

std::string CGlobalConnectivity::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string CGlobalConnectivity::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" + properties().value<std::string>("description");
}

/////////////////////////////////////////////////////////////////////////////

void CGlobalConnectivity::execute()
{
  CMesh& mesh = *m_mesh.lock();

  Geometry& nodes = mesh.geometry();
  CList<Uint>& nodes_glb_idx = nodes.glb_idx();
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
  CNodeElementConnectivity& node2elem = *mesh.geometry().create_component_ptr<CNodeElementConnectivity>("node2elem");
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
  Component::Ptr elem_comp;
  Uint elem_idx;

  Uint cnt(0);
  for (Uint i=0; i<mesh.geometry().size(); ++i)
  {
    if (mesh.geometry().is_ghost(i))
    {
      ghostnode_glb_idx[cnt] = nodes_glb_idx[i];

      CDynTable<Uint>::ConstRow elems = node2elem.connectivity()[i];
      boost_foreach(const Uint e, elems)
      {
        boost::tie(elem_comp,elem_idx) = node2elem.elements().location(e);
        ghostnode_glb_elem_connectivity.push_back(elem_comp->as_type<CElements>().glb_idx()[elem_idx]);
      }
      ghostnode_glb_elem_connectivity_start[cnt+1] = ghostnode_glb_elem_connectivity_start[cnt] + elems.size();

      ++cnt;
    }
  }

  // 4)
  std::vector<std::vector<Uint> > glb_elem_connectivity(nodes.size());
  nodes_glb_idx.resize(mesh.geometry().size());

  for (Uint root=0; root<Comm::PE::instance().size(); ++root)
  {
    std::vector<Uint> rcv_glb_node_idx(0);//ghostnode_glb_idx.size());
    Comm::PE::instance().broadcast(ghostnode_glb_idx,rcv_glb_node_idx,root);
    std::vector<Uint> rcv_glb_elem_connectivity(0);//ghostnode_glb_elem_connectivity.size());
    Comm::PE::instance().broadcast(ghostnode_glb_elem_connectivity,rcv_glb_elem_connectivity,root);
    std::vector<Uint> rcv_glb_elem_connectivity_start(0);//ghostnode_glb_elem_connectivity_start.size());
    Comm::PE::instance().broadcast(ghostnode_glb_elem_connectivity_start,rcv_glb_elem_connectivity_start,root);

    if (Comm::PE::instance().rank() != root)
    {
      for (Uint p=0; p<Comm::PE::instance().size(); ++p)
      {
        if (p == Comm::PE::instance().rank())
        {
          Uint rcv_idx(0);
          boost_foreach(const std::size_t glb_node, rcv_glb_node_idx)
          {
            if (node_glb2loc.find(glb_node) != node_glb2loc.end())
            {
              //std::cout << "["<<Comm::PE::instance().rank() << "] owns ghostnode " << glb_node << " of [" << root << "]" << std::endl;
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


  CDynTable<Uint>& nodes_glb_elem_connectivity = mesh.geometry().glb_elem_connectivity();
  nodes_glb_elem_connectivity.resize(glb_elem_connectivity.size());
  for (Uint i=0; i<glb_elem_connectivity.size(); ++i)
  {
    CDynTable<Uint>::ConstRow elems = node2elem.connectivity()[i];
    nodes_glb_elem_connectivity[i].resize(glb_elem_connectivity[i].size() + elems.size());
    cnt = 0;
    boost_foreach(const Uint e, elems)
    {
      boost::tie(elem_comp,elem_idx) = node2elem.elements().location(e);
      nodes_glb_elem_connectivity[i][cnt++] = elem_comp->as_type<CElements>().glb_idx()[elem_idx];
    }
    for (Uint j=0; j<glb_elem_connectivity[i].size(); ++j)
    {
      nodes_glb_elem_connectivity[i][cnt++] = glb_elem_connectivity[i][j];
    }

  }
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
