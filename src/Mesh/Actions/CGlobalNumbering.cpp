// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

//#define BOOST_HASH_NO_IMPLICIT_CASTS
#include <boost/functional/hash.hpp>

#include <boost/static_assert.hpp>
#include <set>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"

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

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

  using namespace Common;
  using namespace Math::Functions;
  using namespace Math::Consts;

  create_component_data_type( std::vector<std::size_t> , Mesh_Actions_API , CVector_size_t , "CVector<size_t>" );

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CGlobalNumbering, CMeshTransformer, LibActions> CGlobalNumbering_Builder;

//////////////////////////////////////////////////////////////////////////////

CGlobalNumbering::CGlobalNumbering( const std::string& name )
: CMeshTransformer(name),
  m_debug(false)
{

  m_properties["brief"] = std::string("Construct global node and element numbering based on coordinates hash values");
  std::string desc;
  desc =
    "  Usage: CGlobalNumbering Regions:array[uri]=region1,region2\n\n";
  m_properties["description"] = desc;

  m_options.add_option<OptionT<bool> >("debug", m_debug)
      ->description("Perform checks on validity")
      ->pretty_name("Debug")
      ->link_to(&m_debug);

  m_options.add_option<OptionT<bool> >("combined", true)
      ->description("Combine nodes and elements in one global numbering")
      ->pretty_name("Combined");
}

/////////////////////////////////////////////////////////////////////////////

std::string CGlobalNumbering::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string CGlobalNumbering::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" + properties().value<std::string>("description");
}

/////////////////////////////////////////////////////////////////////////////

void CGlobalNumbering::execute()
{
  CMesh& mesh = *m_mesh.lock();

  CTable<Real>& coordinates = mesh.geometry().coordinates();

  if ( is_null( mesh.geometry().get_child_ptr("glb_node_hash") ) )
    mesh.geometry().create_component<CVector_size_t>("glb_node_hash");
  CVector_size_t& glb_node_hash = mesh.geometry().get_child("glb_node_hash").as_type<CVector_size_t>();
  glb_node_hash.data().resize(coordinates.size());
  Uint i(0);
  boost_foreach(CTable<Real>::ConstRow coords, coordinates.array() )
  {
    glb_node_hash.data()[i]=hash_value(to_vector(coords));
    if (m_debug)
      std::cout << "["<<MPI::PE::instance().rank() << "]  hashing node ("<< to_vector(coords).transpose() << ") to " << glb_node_hash.data()[i] << std::endl;
    ++i;
  }

  boost_foreach( CEntities& elements, find_components_recursively<CEntities>(mesh) )
  {
    RealMatrix element_coordinates(elements.element_type().nb_nodes(),coordinates.row_size());

    if ( is_null( elements.get_child_ptr("glb_elem_hash") ) )
      elements.create_component<CVector_size_t>("glb_elem_hash");
    CVector_size_t& glb_elem_hash = elements.get_child("glb_elem_hash").as_type<CVector_size_t>();
    glb_elem_hash.data().resize(elements.size());

    for (Uint elem_idx=0; elem_idx<elements.size(); ++elem_idx)
    {
      elements.put_coordinates(element_coordinates,elem_idx);
      glb_elem_hash.data()[elem_idx]=hash_value(element_coordinates);
      if (m_debug)
        std::cout << "["<<MPI::PE::instance().rank() << "]  hashing elem ("<< elements.uri().path() << "["<<elem_idx<<"]) to " << glb_elem_hash.data()[elem_idx] << std::endl;

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

    boost_foreach( CEntities& elements, find_components_recursively<CEntities>(mesh) )
    {
      CVector_size_t& glb_elem_hash = elements.get_child("glb_elem_hash").as_type<CVector_size_t>();
      for (Uint i=0; i<glb_elem_hash.data().size(); ++i)
      {
        if (glb_set.insert(glb_elem_hash.data()[i]).second == false)  // it was already in the set
          throw ValueExists(FromHere(), "elem "+elements.uri().path()+"["+to_str(i)+"] is duplicated");
      }
    }

  }


  // now renumber

  //------------------------------------------------------------------------------
  // create node_glb2loc mapping
  Geometry& nodes = mesh.geometry();
  std::map<std::size_t,Uint> node_glb2loc;
  Uint loc_node_idx(0);
  boost_foreach(std::size_t hash, glb_node_hash.data())
    node_glb2loc[hash]=loc_node_idx++;
  std::map<std::size_t,Uint>::iterator node_glb2loc_it;
  std::map<std::size_t,Uint>::iterator hash_not_found = node_glb2loc.end();

  // Check if all nodes have been added to the map
  cf_assert(loc_node_idx==nodes.size());

  //------------------------------------------------------------------------------
  // get tot nb of owned indexes and communicate

  Uint nb_owned_nodes(0);
  CList<Uint>& nodes_rank = mesh.geometry().rank();
  nodes_rank.resize(nodes.size());
  for (Uint i=0; i<nodes.size(); ++i)
  {
    if (nodes.is_ghost(i) == false)
    {
      ++nb_owned_nodes;
      nodes_rank[i] = MPI::PE::instance().rank();
    }
  }

  Uint nb_owned_elems(0);
  boost_foreach( CEntities& elements, find_components_recursively<CEntities>(mesh) )
  {
    CList<Uint>& elem_rank = elements.rank();
    elem_rank.resize(elements.size());

    for (Uint e=0; e<elements.size(); ++e)
    {
      if (elements.is_ghost(e) == false)
      {
        ++nb_owned_elems;
        elem_rank[e] = MPI::PE::instance().rank();
      }
    }
  }

  Uint tot_nb_owned_ids=nb_owned_nodes + nb_owned_elems;

  std::vector<Uint> nb_ids_per_proc(MPI::PE::instance().size());
  MPI::PE::instance().all_gather(tot_nb_owned_ids, nb_ids_per_proc);
  std::vector<Uint> start_id_per_proc(MPI::PE::instance().size());
  Uint start_id=0;
  for (Uint p=0; p<nb_ids_per_proc.size(); ++p)
  {
    start_id_per_proc[p] = start_id;
    start_id += nb_ids_per_proc[p];
  }

  //------------------------------------------------------------------------------
  // add glb_idx to owned nodes, broadcast/receive glb_idx for ghost nodes

  std::vector<size_t> node_from(nb_owned_nodes);
  std::vector<Uint>   node_to(nb_owned_nodes);

  CList<Uint>& nodes_glb_idx = mesh.geometry().glb_idx();
  nodes_glb_idx.resize(nodes.size());

  Uint cnt=0;
  Uint glb_id = start_id_per_proc[MPI::PE::instance().rank()];
  for (Uint i=0; i<nodes.size(); ++i)
  {
    if ( ! nodes.is_ghost(i) )
    {
      nodes_glb_idx[i] = glb_id++;
      node_from[cnt] = glb_node_hash.data()[i];
      node_to[cnt]   = nodes_glb_idx[i];
      ++cnt;
    }
    else
    {
      nodes_glb_idx[i] = uint_max();
    }
  }

  for (Uint root=0; root<MPI::PE::instance().size(); ++root)
  {
    std::vector<std::size_t> rcv_node_from(0);
    MPI::PE::instance().broadcast(node_from,rcv_node_from,root);
    std::vector<Uint>        rcv_node_to(0);
    MPI::PE::instance().broadcast(node_to,rcv_node_to,root);
    if (MPI::PE::instance().rank() != root)
    {
      for (Uint p=0; p<MPI::PE::instance().size(); ++p)
      {
        if (p == MPI::PE::instance().rank())
        {
          Uint rcv_idx(0);
          boost_foreach(const std::size_t node_hash, rcv_node_from)
          {
            node_glb2loc_it = node_glb2loc.find(node_hash);
            if ( node_glb2loc_it != hash_not_found )
            {
              if (m_debug)
                std::cout << "["<<MPI::PE::instance().rank() << "]  will change node "<< node_glb2loc_it->first << " (" << node_glb2loc_it->second << ") to " << rcv_node_to[rcv_idx] << std::endl;
              nodes_glb_idx[node_glb2loc_it->second]=rcv_node_to[rcv_idx];
              cf_assert(nodes.is_ghost(node_glb2loc_it->second));
              nodes_rank[node_glb2loc_it->second]=root;
            }
            ++rcv_idx;
          }
          break;
        }
      }
    }
  }

  if (m_debug)
  {
    for (Uint i=0; i<nodes.size(); ++i)
    {
      cf_assert(nodes.glb_idx()[i] != uint_max());
      if (nodes.is_ghost(i) == false)
      {
        cf_assert(nodes.glb_idx()[i] >= start_id_per_proc[MPI::PE::instance().rank()]);
        cf_assert(nodes.glb_idx()[i] < start_id_per_proc[MPI::PE::instance().rank()] + nb_owned_nodes);
      }
    }
  }

  //------------------------------------------------------------------------------
  // give glb idx to elements

  boost_foreach( CEntities& elements, find_components_recursively<CEntities>(mesh) )
  {
    std::vector<std::size_t>& glb_elem_hash = elements.get_child("glb_elem_hash").as_type<CVector_size_t>().data();
    CList<Uint>& elem_rank = elements.rank();
    elem_rank.resize(elements.size());

    //------------------------------------------------------------------------------
    // create elem_glb2loc mapping
    Uint nb_owned_elems=0;
    std::map<std::size_t,Uint> elem_glb2loc;
    std::map<std::size_t,Uint>::iterator elem_glb2loc_it;
    std::map<std::size_t,Uint>::iterator hash_not_found = elem_glb2loc.end();

    for (Uint e=0; e<elements.size(); ++e)
    {
      elem_glb2loc [ glb_elem_hash[e] ] = e;
      if ( ! elements.is_ghost(e) )
        ++nb_owned_elems;
    }


    std::vector<size_t> send_hash(nb_owned_elems);
    std::vector<Uint>   send_id(nb_owned_elems);

    CList<Uint>& elements_glb_idx = elements.glb_idx();
    elements_glb_idx.resize(elements.size());
    cf_assert(glb_elem_hash.size() == elements.size());

    Uint cnt(0);
    for (Uint e=0; e<elements.size(); ++e)
    {
      if (m_debug)
        std::cout << "["<<MPI::PE::instance().rank() << "]  will change elem "<< glb_elem_hash[e] << " (" << elements.uri().path() << "["<<e<<"]) to " << glb_id << std::endl;

      if ( ! elements.is_ghost(e) )
      {
        elements_glb_idx[e] = glb_id++;
        send_hash[cnt] = glb_elem_hash[e];
        send_id[cnt]   = elements_glb_idx[e];
        ++cnt;
      }
      else
      {
        elements_glb_idx[e] = uint_max();
      }
    } // end foreach elem_idx
    cf_assert(cnt == nb_owned_elems);

    for (Uint root=0; root<MPI::PE::instance().size(); ++root)
    {
      std::vector<std::size_t> recv_hash(0);
      MPI::PE::instance().broadcast(send_hash,recv_hash,root);
      std::vector<Uint>        recv_id(0);
      MPI::PE::instance().broadcast(send_id,recv_id,root);
      if (MPI::PE::instance().rank() != root)
      {
        for (Uint p=0; p<MPI::PE::instance().size(); ++p)
        {
          if (p == MPI::PE::instance().rank())
          {
            Uint recv_idx(0);
            boost_foreach(const std::size_t hash, recv_hash)
            {
              elem_glb2loc_it = elem_glb2loc.find(hash);
              if ( elem_glb2loc_it != hash_not_found )
              {
                if (m_debug)
                  std::cout << "["<<MPI::PE::instance().rank() << "]  will change elem "<< elem_glb2loc_it->first << " (" << elem_glb2loc_it->second << ") to " << recv_id[recv_idx] << std::endl;
                elements_glb_idx[elem_glb2loc_it->second]=recv_id[recv_idx];
                cf_assert(elements.is_ghost(elem_glb2loc_it->second));
                elem_rank[elem_glb2loc_it->second]=root;
              }
              ++recv_idx;
            }
            break;
          }
        }
      }
    } // end foreach broadcasting process



  } // end foreach elements


  // In debug mode, check if no hashes are duplicated
  if (m_debug)
  {
    std::set<Uint> glb_set;
    for (Uint i=0; i<nodes_glb_idx.size(); ++i)
    {
      if (glb_set.insert(nodes_glb_idx[i]).second == false)  // it was already in the set
        throw ValueExists(FromHere(), "node "+to_str(i)+" is duplicated");
    }

    boost_foreach( CEntities& elements, find_components_recursively<CEntities>(mesh) )
    {
      CList<Uint>& elements_glb_idx = elements.glb_idx();
      for (Uint i=0; i<elements.size(); ++i)
      {
        if (glb_set.insert(elements_glb_idx[i]).second == false)  // it was already in the set
          throw ValueExists(FromHere(), "elem "+elements.uri().path()+"["+to_str(i)+"] is duplicated");
      }
    }
  }


  mesh.geometry().remove_component(glb_node_hash);

  boost_foreach( CEntities& elements, find_components_recursively<CEntities>(mesh) )
  {
    elements.remove_component("glb_elem_hash");
  }


}

////////////////////////////////////////////////////////////////////////////////

std::size_t CGlobalNumbering::hash_value(const RealMatrix& coords)
{
  std::size_t seed=0;
  for (Uint i=0; i<coords.rows(); ++i)
  for (Uint j=0; j<coords.cols(); ++j)
  {
    // multiply with 1e-5 (arbitrary) to avoid hash collisions
    boost::hash_combine(seed,1e-3*coords(i,j));
  }
  return seed;
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
