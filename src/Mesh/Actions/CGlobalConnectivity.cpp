// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpi/collectives.hpp>

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
#include "Mesh/CNodeElementConnectivity.hpp"

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
  
  std::vector<std::size_t>& glb_node_hash = mesh.nodes().get_child("glb_node_hash").as_type<CVector_size_t>().data();
  index_foreach(loc_node_idx, std::size_t glb_node_idx, glb_node_hash)
  {
    node_glb2loc[glb_node_idx]=loc_node_idx;
  }
  
  //2)
  CList<bool>& is_node_ghost = mesh.nodes().is_ghost();
  std::vector<std::vector<std::size_t> > glb_elem_connectivity(mesh.nodes().size()); 
  // boost_foreach( CElements& elements, find_components_recursively<CElements>(mesh) )
  // {
  //   CTable<Uint>& node_connectivity = elements.connectivity_table();
  //   std::vector<std::size_t>& glb_elm_hash = elements.get_child("glb_elem_hash").as_type<CVector_size_t>().data();
  //   for (Uint elm_idx=0; elm_idx<elements.size(); ++elm_idx)
  //   {
  //     const Uint glb_elm_idx = glb_elm_hash[elm_idx];
  //     boost_foreach(Uint node_idx, node_connectivity[elm_idx])
  //       glb_elem_connectivity[node_idx].push_back(glb_elm_idx);
  //   }
  // }
  CNodeElementConnectivity& node2elem = *mesh.nodes().create_component<CNodeElementConnectivity>("node2elem");
  node2elem.setup(mesh.topology());
  
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
  std::vector<std::size_t> ghostnode_glb_elem_connectivity_start(nb_ghost+1);
  Uint cnt(0);
  ghostnode_glb_elem_connectivity_start[cnt]=0;
  CElements::Ptr elem_comp;
  Uint elem_idx;
  
  for (Uint i=0; i<mesh.nodes().size(); ++i)
  {
    if (mesh.nodes().is_ghost()[i])
    {
      ghostnode_glb_idx[cnt] = glb_node_hash[i];
      
      CDynTable<Uint>::ConstRow elems = node2elem.elements(i);
      boost_foreach(const Uint e, elems)
      {
        boost::tie(elem_comp,elem_idx) = node2elem.element_location(e);
        ghostnode_glb_elem_connectivity.push_back(elem_comp->get_child("glb_elem_hash").as_type<CVector_size_t>().data()[elem_idx]);
      }
      ghostnode_glb_elem_connectivity_start[cnt+1] = ghostnode_glb_elem_connectivity_start[cnt] + elems.size();
            
      ++cnt;
    }
  }

  // 4)
  for (Uint root=0; root<mpi::PE::instance().size(); ++root)
  {
    std::vector<std::size_t> rcv_glb_node_idx = mpi::broadcast(ghostnode_glb_idx, root);
    std::vector<std::size_t> rcv_glb_elem_connectivity = mpi::broadcast(ghostnode_glb_elem_connectivity, root);
    std::vector<std::size_t> rcv_glb_elem_connectivity_start = mpi::broadcast(ghostnode_glb_elem_connectivity_start, root);
    
    if (mpi::PE::instance().rank() != root)
    {
      for (Uint p=0; p<mpi::PE::instance().size(); ++p)
      {
        if (p == mpi::PE::instance().rank())
        {
          boost::this_thread::sleep(boost::posix_time::milliseconds(5));
          Uint rcv_idx(0);
          boost_foreach(const std::size_t glb_node, rcv_glb_node_idx)
          {
            if (node_glb2loc.find(glb_node) != node_glb2loc.end())
            {
              std::cout << "["<<mpi::PE::instance().rank() << "] owns ghostnode " << glb_node << " of [" << root << "]" << std::endl;
              
              Uint loc_node_idx = node_glb2loc[glb_node];
              for(Uint l=rcv_glb_elem_connectivity_start[rcv_idx]; l<rcv_glb_elem_connectivity_start[rcv_idx+1]; ++l)
                glb_elem_connectivity[loc_node_idx].push_back(rcv_glb_elem_connectivity[l]);
            }
            ++rcv_idx;
          }
        }
      }
    }
  }
  
  // glb_elem_connectivity is now complete!
  for (Uint i=0; i<glb_elem_connectivity.size(); ++i)
  {
    CFinfo << i << " :   "; 
    for (Uint j=0; j<glb_elem_connectivity[i].size(); ++j)
    {
      CFinfo << glb_elem_connectivity[i][j] << " ";
    }
    CFinfo << CFendl;
  }
  
  // glb_node_connectivity is available as  nodes.glb_node_hash(elements.connectivity_table())
  
  
  // now renumber
  Uint tot_nb_owned_ids=mesh.nodes().size()-nb_ghost;
  boost_foreach( CElements& elements, find_components_recursively<CElements>(mesh) )
  {
    tot_nb_owned_ids += elements.size();
  }
  std::vector<Uint> nb_ids_per_proc(mpi::PE::instance().size());
  /// @todo replace boost::mpi::communicator by CF instructions
  boost::mpi::communicator world;
  boost::mpi::all_gather(world, tot_nb_owned_ids, nb_ids_per_proc);
  std::vector<Uint> start_id_per_proc(mpi::PE::instance().size());
  Uint start_id=0;
  for (Uint p=0; p<nb_ids_per_proc.size(); ++p)
  {
    start_id_per_proc[p] = start_id;
    start_id += nb_ids_per_proc[p];
  }  
  
  CFLogVar(to_vector(start_id_per_proc).transpose());
  
  std::vector<size_t> node_from(mesh.nodes().size()-nb_ghost);
  std::vector<Uint>   node_to(mesh.nodes().size()-nb_ghost);

  std::vector<size_t> elem_from(tot_nb_owned_ids-node_from.size());
  std::vector<Uint>   elem_to(tot_nb_owned_ids-node_to.size());
  
  CList<Uint>& node_new_idx = mesh.nodes().glb_idx();
  CList<bool>& node_is_ghost = mesh.nodes().is_ghost();
  
  cnt=0;
  Uint glb_id = start_id_per_proc[mpi::PE::instance().rank()];
  for (Uint i=0; i<mesh.nodes().size(); ++i)
  {
    if ( ! node_is_ghost[i] )
    {
      node_new_idx[i] = glb_id++;
      node_from[cnt] = glb_node_hash[i];
      node_to[cnt]   = node_new_idx[i];
      ++cnt;
    }
  }
  cnt=0;
  boost_foreach( CElements& elements, find_components_recursively<CElements>(mesh) )
  {
    CList<Uint>& glb_elem_idx = elements.glb_idx();
    Uint e(0);
    boost_foreach( std::size_t glb_elm_hash, elements.get_child("glb_elem_hash").as_type<CVector_size_t>().data())
    {
      glb_elem_idx[e++] = glb_id;
      elem_from[cnt] = glb_elm_hash;
      elem_to[cnt] = glb_id;
      ++cnt;
      ++glb_id;
    }
  }  
  
  for (Uint root=0; root<mpi::PE::instance().size(); ++root)
  {
    {
      std::vector<std::size_t> rcv_node_from = mpi::broadcast(node_from, root);
      std::vector<Uint>        rcv_node_to   = mpi::broadcast(node_to, root);
      std::map<std::size_t,Uint> change;
      for (Uint i=0; i<rcv_node_from.size(); ++i) 
        change[rcv_node_from[i]]=rcv_node_to[i];
      if (mpi::PE::instance().rank() != root)
      {
        for (Uint p=0; p<mpi::PE::instance().size(); ++p)
        {
          if (p == mpi::PE::instance().rank())
          {
            boost::this_thread::sleep(boost::posix_time::milliseconds(5));
            Uint rcv_idx(0);
            boost_foreach(const std::size_t node_from_id, rcv_node_from)
            {
              if (node_glb2loc.find(node_from_id) != node_glb2loc.end())
              {
                std::cout << "["<<mpi::PE::instance().rank() << "] will change node " << node_from_id << " to " << rcv_node_to[rcv_idx] << std::endl;
              }
              ++rcv_idx;
            }
          }
        }
      }
    }
    // broadcast elements
    {
      std::vector<std::size_t> rcv_elem_from = mpi::broadcast(elem_from, root);
      std::vector<Uint>        rcv_elem_to   = mpi::broadcast(elem_to, root);
      
      for (Uint p=0; p<mpi::PE::instance().size(); ++p)
      {
        if (p == mpi::PE::instance().rank())
        {
          boost::this_thread::sleep(boost::posix_time::milliseconds(5));
          std::cout << "["<<p<<"] rcv ";
          for (Uint i=0; i<rcv_elem_from.size(); ++i)
            std::cout << rcv_elem_from[i] << " " ;
          std::cout << std::endl;
        }
      }
      
      std::map<std::size_t,Uint> change;
      for (Uint i=0; i<rcv_elem_from.size(); ++i)
        change[rcv_elem_from[i]]=rcv_elem_to[i];
      std::map<std::size_t,Uint>::iterator a_change;
      std::map<std::size_t,Uint>::iterator not_found(change.end());
      rcv_elem_from.clear();
      rcv_elem_to.clear();
      if (mpi::PE::instance().rank() != root)
      {
        for (Uint p=0; p<mpi::PE::instance().size(); ++p)
        {
          if (p == mpi::PE::instance().rank())
          {
            boost::this_thread::sleep(boost::posix_time::milliseconds(5));
          
            for (Uint node_idx=0; node_idx<glb_elem_connectivity.size(); ++node_idx)
            {
              for (Uint elem_idx=0; elem_idx<glb_elem_connectivity[node_idx].size(); ++elem_idx)
              {
                a_change = change.find(glb_elem_connectivity[node_idx][elem_idx]);
                if ( a_change != not_found )
                {
                  glb_elem_connectivity[node_idx][elem_idx] = a_change->second;                  
                }
              }
            }
          }
        }
      }
    }
    
  }
  
  for (Uint i=0; i<glb_elem_connectivity.size(); ++i)
  {
    CFinfo << i << " :   "; 
    for (Uint j=0; j<glb_elem_connectivity[i].size(); ++j)
    {
      CFinfo << glb_elem_connectivity[i][j] << " ";
    }
    CFinfo << CFendl;
  }
  
  // fill in missing entries in glb_elem_connectivity
  for (Uint i=0; i<mesh.nodes().size(); ++i)
  {
    if (!is_node_ghost[i])
    {
      CDynTable<Uint>::ConstRow elems = node2elem.elements(i);
      boost_foreach(const Uint e, elems)
      {
        boost::tie(elem_comp,elem_idx) = node2elem.element_location(e);
        glb_elem_connectivity[i].push_back(elem_comp->glb_idx()[elem_idx]);
      }
    }
  }
  
  CFinfo << CFendl;
  for (Uint i=0; i<glb_elem_connectivity.size(); ++i)
  {
    CFinfo << mesh.nodes().glb_idx()[i] << " :   "; 
    for (Uint j=0; j<glb_elem_connectivity[i].size(); ++j)
    {
      CFinfo << glb_elem_connectivity[i][j] << " ";
    }
    CFinfo << CFendl;
  }
  
  CDynTable<Uint>& nodes_glb_elem_connectivity = mesh.nodes().glb_elem_connectivity();
  nodes_glb_elem_connectivity.resize(glb_elem_connectivity.size());
  for (Uint i=0; i<glb_elem_connectivity.size(); ++i)
  {
    nodes_glb_elem_connectivity[i].resize(glb_elem_connectivity[i].size());
    for (Uint j=0; j<glb_elem_connectivity[i].size(); ++j)
      nodes_glb_elem_connectivity[i][j] = static_cast<Uint>(glb_elem_connectivity[i][j]);
  }
  
  CFinfo << "Connectivity successful" << CFendl;
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
