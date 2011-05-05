// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>
#include <boost/static_assert.hpp>
#include <set>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/StreamHelpers.hpp"
#include "Common/StringConversion.hpp"
#include "Common/OptionArray.hpp"
#include "Common/CreateComponentDataType.hpp"
#include "Common/OptionT.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/debug.hpp"

#include "Mesh/Actions/CGlobalNumbering.hpp"
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

Common::ComponentBuilder < CGlobalNumbering, CMeshTransformer, LibActions> CGlobalNumbering_Builder;

//////////////////////////////////////////////////////////////////////////////

CGlobalNumbering::CGlobalNumbering( const std::string& name )
: CMeshTransformer(name),
  m_debug(false)
{
   
  properties()["brief"] = std::string("Construct global node and element numbering based on coordinates hash values");
  std::string desc;
  desc = 
    "  Usage: CGlobalNumbering Regions:array[uri]=region1,region2\n\n";
  properties()["description"] = desc;
  
  properties().add_option<OptionT<bool> >("debug","Debug","Perform checks on validity",m_debug)->link_to(&m_debug);
  
}

/////////////////////////////////////////////////////////////////////////////

std::string CGlobalNumbering::brief_description() const
{
  return properties()["brief"].value<std::string>();
}

/////////////////////////////////////////////////////////////////////////////

  
std::string CGlobalNumbering::help() const
{
  return "  " + properties()["brief"].value<std::string>() + "\n" + properties()["description"].value<std::string>();
}  
  
/////////////////////////////////////////////////////////////////////////////

void CGlobalNumbering::execute()
{
  CMesh& mesh = *m_mesh.lock();

  CTable<Real>& coordinates = mesh.nodes().coordinates();
  CVector_size_t& glb_node_hash = *mesh.nodes().create_component_ptr<CVector_size_t>("glb_node_hash");
  glb_node_hash.data().resize(coordinates.size());
  Uint i(0);
  boost_foreach(CTable<Real>::ConstRow coords, coordinates.array() )
  {    
    glb_node_hash.data()[i]=hash_value(to_vector(coords));
    if (m_debug)
      std::cout << "["<<mpi::PE::instance().rank() << "]  hashing node ("<< to_vector(coords).transpose() << ") to " << glb_node_hash.data()[i] << std::endl;
    ++i;
  }
  
  boost_foreach( CElements& elements, find_components_recursively<CElements>(mesh) )
  {
    RealMatrix element_coordinates(elements.element_type().nb_nodes(),coordinates.row_size());
    CVector_size_t& glb_elem_hash = *elements.create_component_ptr<CVector_size_t>("glb_elem_hash");
    glb_elem_hash.data().resize(elements.size());
    for (Uint elem_idx=0; elem_idx<elements.size(); ++elem_idx)
    {
      elements.put_coordinates(element_coordinates,elem_idx);
      glb_elem_hash.data()[elem_idx]=hash_value(element_coordinates);
      if (m_debug)
        std::cout << "["<<mpi::PE::instance().rank() << "]  hashing elem ("<< elements.full_path().path() << "["<<elem_idx<<"]) to " << glb_elem_hash.data()[elem_idx] << std::endl;
      
      //CFinfo << "glb_elem_hash["<<elem_idx<<"] = " <<  glb_elem_hash.data()[elem_idx] << CFendl;
    }
  }  
  
  
  // In debug mode, check if no hashes are duplicated
  if (m_debug)
  {
    std::set<std::size_t> glb_set;
    for (Uint i=0; i<glb_node_hash.data().size(); ++i)
    {    
      if (glb_set.insert(glb_node_hash.data()[i]).second == false)  // it was already in the set
        throw ValueExists(FromHere(), "node "+to_str(i)+" is duplicated");
    }

    boost_foreach( CElements& elements, find_components_recursively<CElements>(mesh) )
    {
      CVector_size_t& glb_elem_hash = elements.get_child("glb_elem_hash").as_type<CVector_size_t>();
      for (Uint i=0; i<glb_elem_hash.data().size(); ++i)
      {    
        if (glb_set.insert(glb_elem_hash.data()[i]).second == false)  // it was already in the set
          throw ValueExists(FromHere(), "elem "+elements.full_path().path()+"["+to_str(i)+"] is duplicated");
      }
    }

  }
  
  
  // now renumber
  
  //------------------------------------------------------------------------------
  // create node_glb2loc mapping
  std::map<std::size_t,Uint> node_glb2loc;  
  index_foreach(loc_node_idx, std::size_t hash, glb_node_hash.data())
    node_glb2loc[hash]=loc_node_idx;
  std::map<std::size_t,Uint>::iterator node_glb2loc_it;
  std::map<std::size_t,Uint>::iterator hash_not_found = node_glb2loc.end();
  
  
  //------------------------------------------------------------------------------
  // get tot nb of owned indexes and communicate
  
  Uint nb_ghost(0);
  CNodes& nodes = mesh.nodes();
  CList<bool>& nodes_is_ghost = mesh.nodes().is_ghost();
  for (Uint i=0; i<nodes.size(); ++i)
    if (nodes_is_ghost[i])
      ++nb_ghost;
  Uint tot_nb_owned_ids=nodes.size()-nb_ghost;
  boost_foreach( CEntities& elements, find_components_recursively<CElements>(mesh) )
    tot_nb_owned_ids += elements.size();
    
  std::vector<Uint> nb_ids_per_proc(mpi::PE::instance().size());
  //boost::mpi::communicator world;
  //boost::mpi::all_gather(world, tot_nb_owned_ids, nb_ids_per_proc);
  mpi::PE::instance().all_gather(tot_nb_owned_ids, nb_ids_per_proc);
  std::vector<Uint> start_id_per_proc(mpi::PE::instance().size());
  Uint start_id=0;
  for (Uint p=0; p<nb_ids_per_proc.size(); ++p)
  {
    start_id_per_proc[p] = start_id;
    start_id += nb_ids_per_proc[p];
  }
  
  
  //------------------------------------------------------------------------------
  // add glb_idx to owned nodes, broadcast/receive glb_idx for ghost nodes
  
  std::vector<size_t> node_from(nodes.size()-nb_ghost);
  std::vector<Uint>   node_to(nodes.size()-nb_ghost);
  
  CList<Uint>& nodes_glb_idx = mesh.nodes().glb_idx();
  nodes_glb_idx.resize(nodes.size());
  
  Uint cnt=0;
  Uint glb_id = start_id_per_proc[mpi::PE::instance().rank()];
  for (Uint i=0; i<nodes.size(); ++i)
  {
    if ( ! nodes_is_ghost[i] )
    {
      nodes_glb_idx[i] = glb_id++;
      node_from[cnt] = glb_node_hash.data()[i];
      node_to[cnt]   = nodes_glb_idx[i];
      ++cnt;
    }
  }
  
  for (Uint root=0; root<mpi::PE::instance().size(); ++root)
  {
    //std::vector<std::size_t> rcv_node_from = mpi::broadcast(node_from, root);
    //std::vector<Uint>        rcv_node_to   = mpi::broadcast(node_to, root);
    // PECheckPoint(100,"001");
    std::vector<std::size_t> rcv_node_from(0);//node_from.size());
    mpi::PE::instance().broadcast(node_from,rcv_node_from,root);
    //PECheckPoint(100,"002");
    std::vector<Uint>        rcv_node_to(0);//node_to.size());
    mpi::PE::instance().broadcast(node_to,rcv_node_to,root);
    //PECheckPoint(100,"003");
    if (mpi::PE::instance().rank() != root)
    {
      for (Uint p=0; p<mpi::PE::instance().size(); ++p)
      {
        if (p == mpi::PE::instance().rank())
        {
          Uint rcv_idx(0);
          boost_foreach(const std::size_t node_hash, rcv_node_from)
          {            
            node_glb2loc_it = node_glb2loc.find(node_hash);
            if ( node_glb2loc_it != hash_not_found )
            {
              if (m_debug)
                std::cout << "["<<mpi::PE::instance().rank() << "]  will change node "<< node_glb2loc_it->first << " (" << node_glb2loc_it->second << ") to " << rcv_node_to[rcv_idx] << std::endl;
              nodes_glb_idx[node_glb2loc_it->second]=rcv_node_to[rcv_idx];
            }
            ++rcv_idx;
          }
          break;
        }
      }
    }
  }
  
  
  //------------------------------------------------------------------------------
  // give glb idx to elements
  
  boost_foreach( CEntities& elements, find_components_recursively<CElements>(mesh) )
  {
    CList<Uint>& elements_glb_idx = elements.glb_idx();
    elements_glb_idx.resize(elements.size());
    std::vector<std::size_t>& glb_elem_hash = elements.get_child("glb_elem_hash").as_type<CVector_size_t>().data();
    cf_assert(glb_elem_hash.size() == elements.size());
    for (Uint e=0; e<elements.size(); ++e)
    {
      if (m_debug)
        std::cout << "["<<mpi::PE::instance().rank() << "]  will change elem "<< glb_elem_hash[e] << " (" << elements.full_path().path() << "["<<e<<"]) to " << glb_id << std::endl;
      elements_glb_idx[e] = glb_id;
      ++glb_id;
    }
  }
  
  
  // In debug mode, check if no hashes are duplicated
  if (m_debug)
  {
    std::set<Uint> glb_set;
    for (Uint i=0; i<nodes_glb_idx.size(); ++i)
    {    
      if (glb_set.insert(nodes_glb_idx[i]).second == false)  // it was already in the set
        throw ValueExists(FromHere(), "node "+to_str(i)+" is duplicated");
    }

    boost_foreach( CElements& elements, find_components_recursively<CElements>(mesh) )
    {
      CList<Uint>& elements_glb_idx = elements.glb_idx();
      for (Uint i=0; i<elements.size(); ++i)
      {    
        if (glb_set.insert(elements_glb_idx[i]).second == false)  // it was already in the set
          throw ValueExists(FromHere(), "elem "+elements.full_path().path()+"["+to_str(i)+"] is duplicated");
      }
    }

  }
  
  
  CFinfo << "Global Numbering successful" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

std::size_t CGlobalNumbering::hash_value(const RealVector& coords)
{
  std::size_t seed=0;
  for (Uint i=0; i<coords.size(); ++i)
    boost::hash_combine(seed,coords[i]);
  return seed;
}

////////////////////////////////////////////////////////////////////////////////

std::size_t CGlobalNumbering::hash_value(const RealMatrix& coords)
{
  std::size_t seed=0;
  for (Uint i=0; i<coords.rows(); ++i)
  for (Uint j=0; j<coords.cols(); ++j)
    boost::hash_combine(seed,coords(i,j));
  return seed;
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
