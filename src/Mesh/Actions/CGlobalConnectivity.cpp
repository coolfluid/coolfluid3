// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>
#include <boost/static_assert.hpp>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Foreach.hpp"
#include "Common/StreamHelpers.hpp"
#include "Common/StringConversion.hpp"
#include "Common/OptionArray.hpp"
#include "Common/CreateComponentDataType.hpp"

#include "Common/MPI/PE.hpp"
#include "Common/MPI/broadcast.hpp"
#include "Common/MPI/tools.hpp"

#include "Mesh/Actions/CGlobalConnectivity.hpp"
#include "Mesh/CCellFaces.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"
#include "Mesh/CNodeFaceCellConnectivity.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CMesh.hpp"
#include "Math/MathFunctions.hpp"
#include "Math/MathConsts.hpp"
#include "Mesh/ElementData.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {
  
  using namespace Common;
  using namespace Math::MathFunctions;
  using namespace Math::MathConsts;

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
  return properties()["brief"].value<std::string>();
}

/////////////////////////////////////////////////////////////////////////////

  
std::string CGlobalConnectivity::help() const
{
  return "  " + properties()["brief"].value<std::string>() + "\n" + properties()["description"].value<std::string>();
}  
  
/////////////////////////////////////////////////////////////////////////////

void CGlobalConnectivity::execute()
{
  CMesh& mesh = *m_mesh.lock();

  // Undefined behavior if sizeof(Uint) != sizeof(std::size_t)
  // Assert at compile time
  //BOOST_STATIC_ASSERT(sizeof(std::size_t) == sizeof(Uint));

  // 1) make std::map<glb_node_idx,loc_node_idx>
  // 2) Make loc_node -> glb_elems connectivity
  // 3) foreach ghostnode, store connected owned elements
  // 4) broadcast (3) to all, so they can complete their node-elem connectivity
  // 5) possibly also communicate the location of the ghosts.


  //1)
  std::map<Uint,std::size_t> node_glb2loc;
  index_foreach(loc_node_idx, Uint glb_node_idx, mesh.nodes().get_child("glb_node_hash").as_type<CVector_size_t>().data())
    node_glb2loc[glb_node_idx]=loc_node_idx;
  
  //2)
  CList<bool>& is_node_ghost = mesh.nodes().is_ghost();
  std::vector<std::vector<std::size_t> > glb_elem_connectivity(mesh.nodes().size()); 
  boost_foreach( CElements& elements, find_components_recursively<CElements>(mesh) )
  {
    CTable<Uint>& node_connectivity = elements.connectivity_table();
    std::vector<std::size_t>& glb_elm_hash = elements.get_child("glb_elem_hash").as_type<CVector_size_t>().data();
    for (Uint elm_idx=0; elm_idx<elements.size(); ++elm_idx)
    {
      const Uint glb_elm_idx = glb_elm_hash[elm_idx];
      boost_foreach(Uint node_idx, node_connectivity[elm_idx])
        glb_elem_connectivity[node_idx].push_back(glb_elm_idx);
    }
  }
  
  // 3)
  Uint nb_ghost(0);
  for (Uint i=0; i<mesh.nodes().size(); ++i)
  {
    if (mesh.nodes().is_ghost()[i])
    {
      ++nb_ghost;
    }
  }
  
  std::vector<std::size_t> ghostnode_glb_idx(nb_ghost);
  std::vector<std::size_t> ghostnode_glb_elem_connectivity;
  std::vector<std::size_t> ghostnode_glb_elem_connectivity_strides(nb_ghost);
  Uint cnt(0);
  for (Uint i=0; i<mesh.nodes().size(); ++i)
  {
    if (mesh.nodes().is_ghost()[i])
    {
      ghostnode_glb_idx.push_back(mesh.nodes().glb_idx()[i]);
      boost_foreach(const Uint glb_elm_idx, glb_elem_connectivity[i])
        ghostnode_glb_elem_connectivity.push_back(glb_elm_idx);
      ghostnode_glb_elem_connectivity_strides[cnt++]=glb_elem_connectivity[i].size();
    }
  }

  // 4)
  for (Uint root=0; root<mpi::PE::instance().size(); ++root)
  {
    std::vector<std::size_t> rcv_ghostnode_glb_idx = mpi::broadcast(ghostnode_glb_idx, root);
    std::vector<std::size_t> rcv_ghostnode_glb_elem_connectivity = mpi::broadcast(ghostnode_glb_elem_connectivity, root);
    std::vector<std::size_t> rcv_ghostnode_glb_elem_connectivity_strides = mpi::broadcast(ghostnode_glb_elem_connectivity_strides, root);
  }
  
  CFinfo << "Connectivity successful" << CFendl;
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
