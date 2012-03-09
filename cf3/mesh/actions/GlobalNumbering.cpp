// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

//#define BOOST_HASH_NO_IMPLICIT_CASTS
#include <boost/functional/hash.hpp>

#include <boost/static_assert.hpp>
#include <set>

#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/StreamHelpers.hpp"
#include "common/StringConversion.hpp"
#include "common/OptionArray.hpp"
#include "common/CreateComponentDataType.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionT.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

#include "math/MatrixTypesConversion.hpp"
#include "math/Hilbert.hpp"

#include "mesh/actions/GlobalNumbering.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/FaceCellConnectivity.hpp"
#include "mesh/NodeElementConnectivity.hpp"
#include "mesh/Node2FaceCellConnectivity.hpp"
#include "mesh/Space.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/BoundingBox.hpp"
#include "math/Functions.hpp"
#include "math/Consts.hpp"
#include "mesh/ElementData.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;
  using namespace math::Functions;
  using namespace math::Consts;

  create_component_data_type( std::vector<std::size_t> , mesh_actions_API , CVector_size_t , "CVector<size_t>" );

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < GlobalNumbering, MeshTransformer, mesh::actions::LibActions> GlobalNumbering_Builder;

//////////////////////////////////////////////////////////////////////////////

GlobalNumbering::GlobalNumbering( const std::string& name )
: MeshTransformer(name),
  m_debug(false)
{

  properties()["brief"] = std::string("Construct global node and element numbering based on coordinates hash values");
  std::string desc;
  desc =
    "  Usage: GlobalNumbering Regions:array[uri]=region1,region2\n\n";
  properties()["description"] = desc;

  options().add_option("debug", m_debug)
      .description("Perform checks on validity")
      .pretty_name("Debug")
      .link_to(&m_debug);

  options().add_option("combined", true)
      .description("Combine nodes and elements in one global numbering")
      .pretty_name("Combined");
}

/////////////////////////////////////////////////////////////////////////////

std::string GlobalNumbering::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string GlobalNumbering::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" + properties().value<std::string>("description");
}

/////////////////////////////////////////////////////////////////////////////

void GlobalNumbering::execute()
{
  Mesh& mesh = *m_mesh;

  Handle<BoundingBox> global_bounding_box;

  if (Handle<Component> found = mesh.get_child("global_bounding_box"))
  {
    global_bounding_box = found->handle<BoundingBox>();
  }
  if ( is_null(global_bounding_box)  )
  {
    global_bounding_box = mesh.create_component<BoundingBox>("global_bounding_box");
    global_bounding_box->build(mesh);
    global_bounding_box->make_global();
    global_bounding_box->update_properties();
  }

  math::Hilbert compute_glb_idx(*global_bounding_box,20);

//  PEProcessSortedExecute(-1,
//    boost_foreach( Entities& elements, find_components_recursively<Entities>(mesh) )
//    {
//                         std::cout << PERank << elements.uri() << "rank = \n" << elements.rank() << std::endl;
//    }
//                         )

  if (PE::Comm::instance().size()==1)
  {
    Uint glb_idx=0;
    mesh.geometry_fields().rank().resize(mesh.geometry_fields().size());
    mesh.geometry_fields().glb_idx().resize(mesh.geometry_fields().size());
    for (Uint n=0; n<mesh.geometry_fields().size(); ++n)
    {
      mesh.geometry_fields().rank()[n]=PE::Comm::instance().rank();
      mesh.geometry_fields().glb_idx()[n]=glb_idx++;
    }
    boost_foreach( Entities& elements, find_components_recursively<Entities>(mesh) )
    {
      elements.rank().resize(elements.size());
      elements.glb_idx().resize(elements.size());
      for (Uint e=0; e<elements.size(); ++e)
      {
        elements.rank()[e]=PE::Comm::instance().rank();
        elements.glb_idx()[e]=glb_idx++;
      }
    }
    return;
  }

  common::Table<Real>& coordinates = mesh.geometry_fields().coordinates();
  RealVector coord_vec(coordinates.row_size());

  if ( is_null( mesh.geometry_fields().get_child("glb_node_hash") ) )
    mesh.geometry_fields().create_component<CVector_size_t>("glb_node_hash");
  CVector_size_t& glb_node_hash = *Handle<CVector_size_t>(mesh.geometry_fields().get_child("glb_node_hash"));
  glb_node_hash.data().resize(coordinates.size());
  Uint i(0);
  boost_foreach(common::Table<Real>::ConstRow coords, coordinates.array() )
  {
//    glb_node_hash.data()[i]=node_hash_value(to_vector(coords));
    math::copy(coords,coord_vec);
    glb_node_hash.data()[i]=compute_glb_idx(coord_vec);

    if (m_debug)
      std::cout << "["<<PE::Comm::instance().rank() << "]  hashing node ("<< to_vector(coords).transpose() << ") to " << glb_node_hash.data()[i] << std::endl;
    ++i;
  }

  boost_foreach( Entities& elements, find_components_recursively<Entities>(mesh) )
  {
    RealMatrix element_coordinates(elements.element_type().nb_nodes(),coordinates.row_size());

    if ( is_null( elements.get_child("glb_elem_hash") ) )
      elements.create_component<CVector_size_t>("glb_elem_hash");
    CVector_size_t& glb_elem_hash = *Handle<CVector_size_t>(elements.get_child("glb_elem_hash"));
    glb_elem_hash.data().resize(elements.size());

    for (Uint elem_idx=0; elem_idx<elements.size(); ++elem_idx)
    {
      elements.geometry_space().put_coordinates(element_coordinates,elem_idx);
      RealVector centroid(elements.element_type().dimension());
      elements.element_type().compute_centroid(element_coordinates,centroid);
//      glb_elem_hash.data()[elem_idx]=elem_hash_value(element_coordinates);
      glb_elem_hash.data()[elem_idx]=compute_glb_idx(centroid);
      if (m_debug)
        std::cout << "["<<PE::Comm::instance().rank() << "]  hashing elem "<< elements.uri().path() << "["<<elem_idx<<"] ("<<centroid.transpose()<<") to " << glb_elem_hash.data()[elem_idx] << std::endl;

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

    glb_set.clear();
    boost_foreach( Entities& elements, find_components_recursively<Entities>(mesh) )
    {
      CVector_size_t& glb_elem_hash = *Handle<CVector_size_t>(elements.get_child("glb_elem_hash"));
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
  Dictionary& nodes = mesh.geometry_fields();
  std::map<std::size_t,Uint> node_glb2loc;
  Uint loc_node_idx(0);
  boost_foreach(std::size_t hash, glb_node_hash.data())
    node_glb2loc[hash]=loc_node_idx++;
  std::map<std::size_t,Uint>::iterator node_glb2loc_it;
  std::map<std::size_t,Uint>::iterator hash_not_found = node_glb2loc.end();

  // Check if all nodes have been added to the map
  cf3_assert(loc_node_idx==nodes.size());

  //------------------------------------------------------------------------------
  // get tot nb of owned indexes and communicate

  Uint nb_owned_nodes(0);
  common::List<Uint>& nodes_rank = mesh.geometry_fields().rank();
  nodes_rank.resize(nodes.size());
  for (Uint i=0; i<nodes.size(); ++i)
  {
    if (nodes.is_ghost(i) == false)
    {
      ++nb_owned_nodes;
      nodes_rank[i] = PE::Comm::instance().rank();
    }
  }

  Uint nb_owned_elems(0);
  boost_foreach( Entities& elements, find_components_recursively<Entities>(mesh) )
  {
    common::List<Uint>& elem_rank = elements.rank();
    elem_rank.resize(elements.size());

    for (Uint e=0; e<elements.size(); ++e)
    {
      if (elements.is_ghost(e) == false)
      {
        ++nb_owned_elems;
        elem_rank[e] = PE::Comm::instance().rank();
      }
    }
  }

  Uint tot_nb_owned_ids=nb_owned_nodes + nb_owned_elems;

  std::vector<Uint> nb_ids_per_proc(PE::Comm::instance().size());
  PE::Comm::instance().all_gather(tot_nb_owned_ids, nb_ids_per_proc);
  std::vector<Uint> start_id_per_proc(PE::Comm::instance().size());
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

  common::List<Uint>& nodes_glb_idx = mesh.geometry_fields().glb_idx();
  nodes_glb_idx.resize(nodes.size());

  Uint cnt=0;
  Uint glb_id = start_id_per_proc[PE::Comm::instance().rank()];
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

  for (Uint root=0; root<PE::Comm::instance().size(); ++root)
  {
    std::vector<std::size_t> rcv_node_from(0);
    PE::Comm::instance().broadcast(node_from,rcv_node_from,root);
    std::vector<Uint>        rcv_node_to(0);
    PE::Comm::instance().broadcast(node_to,rcv_node_to,root);
    if (PE::Comm::instance().rank() != root)
    {
      for (Uint p=0; p<PE::Comm::instance().size(); ++p)
      {
        if (p == PE::Comm::instance().rank())
        {
          Uint rcv_idx(0);
          boost_foreach(const std::size_t node_hash, rcv_node_from)
          {
            node_glb2loc_it = node_glb2loc.find(node_hash);
            if ( node_glb2loc_it != hash_not_found )
            {
              if (m_debug)
                std::cout << "["<<PE::Comm::instance().rank() << "]  will change node "<< node_glb2loc_it->first << " (" << node_glb2loc_it->second << ") to " << rcv_node_to[rcv_idx] << std::endl;
              nodes_glb_idx[node_glb2loc_it->second]=rcv_node_to[rcv_idx];
              cf3_assert(nodes.is_ghost(node_glb2loc_it->second));
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
      cf3_assert(nodes.glb_idx()[i] != uint_max());
      if (nodes.is_ghost(i) == false)
      {
        cf3_assert(nodes.glb_idx()[i] >= start_id_per_proc[PE::Comm::instance().rank()]);
        cf3_assert(nodes.glb_idx()[i] < start_id_per_proc[PE::Comm::instance().rank()] + nb_owned_nodes);
      }
    }
  }

  //------------------------------------------------------------------------------
  // give glb idx to elements

  boost_foreach( Entities& elements, find_components_recursively<Entities>(mesh) )
  {
    if (m_debug)
      std::cout << "give glb idx to elements " << elements.uri() << std::endl;
    std::vector<std::size_t>& glb_elem_hash = Handle<CVector_size_t>(elements.get_child("glb_elem_hash"))->data();
    common::List<Uint>& elem_rank = elements.rank();
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

    common::List<Uint>& elements_glb_idx = elements.glb_idx();
    elements_glb_idx.resize(elements.size());
    cf3_assert(glb_elem_hash.size() == elements.size());

    Uint cnt(0);
    for (Uint e=0; e<elements.size(); ++e)
    {

      if ( ! elements.is_ghost(e) )
      {
        if (m_debug)
          std::cout << "["<<PE::Comm::instance().rank() << "]  will change owned elem "<< glb_elem_hash[e] << " (" << elements.uri().path() << "["<<e<<"]) to " << glb_id << std::endl;

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
    cf3_assert(cnt == nb_owned_elems);

    for (Uint root=0; root<PE::Comm::instance().size(); ++root)
    {
      std::vector<std::size_t> recv_hash(0);
      PE::Comm::instance().broadcast(send_hash,recv_hash,root);
      std::vector<Uint>        recv_id(0);
      PE::Comm::instance().broadcast(send_id,recv_id,root);
      if (PE::Comm::instance().rank() != root)
      {
        for (Uint p=0; p<PE::Comm::instance().size(); ++p)
        {
          if (p == PE::Comm::instance().rank())
          {
            Uint recv_idx(0);
            boost_foreach(const std::size_t hash, recv_hash)
            {
              elem_glb2loc_it = elem_glb2loc.find(hash);
              if ( elem_glb2loc_it != hash_not_found )
              {
                if (m_debug)
                  std::cout << "["<<PE::Comm::instance().rank() << "]  will change ghost elem "<< elem_glb2loc_it->first << " (" << elements.uri() << "[" << elem_glb2loc_it->second << "]) to " << recv_id[recv_idx] << std::endl;
                elements_glb_idx[elem_glb2loc_it->second]=recv_id[recv_idx];
                cf3_assert(elements.is_ghost(elem_glb2loc_it->second));
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
      if (nodes_glb_idx[i] == uint_max())
        throw BadValue(FromHere(), "node " + to_str(i)+" doesn't have glb_idx");
    }

    boost_foreach( Entities& elements, find_components_recursively<Entities>(mesh) )
    {
      common::List<Uint>& elements_glb_idx = elements.glb_idx();
      for (Uint i=0; i<elements.size(); ++i)
      {
        if (glb_set.insert(elements_glb_idx[i]).second == false)  // it was already in the set
          throw ValueExists(FromHere(), "elem "+elements.uri().path()+"["+to_str(i)+"] is duplicated");
        if (elements_glb_idx[i] == uint_max())
          throw BadValue(FromHere(), "elem "+elements.uri().path()+"["+to_str(i)+"] doesn't have glb_idx");

      }
    }
  }


  mesh.geometry_fields().remove_component(glb_node_hash);

  boost_foreach( Entities& elements, find_components_recursively<Entities>(mesh) )
  {
    elements.remove_component("glb_elem_hash");
  }
}

////////////////////////////////////////////////////////////////////////////////

//std::size_t GlobalNumbering::node_hash_value(const RealMatrix& coords)
//{
//  std::size_t seed=0;
//  for (Uint i=0; i<coords.rows(); ++i)
//  for (Uint j=0; j<coords.cols(); ++j)
//  {
//    // multiply with 1e-5 (arbitrary) to avoid hash collisions
//    boost::hash_combine(seed,1e-3*coords(i,j));
//  }
//  return seed;
//}

//std::size_t GlobalNumbering::elem_hash_value(const RealMatrix& coords)
//{
//  std::size_t seed=123456789;
//  for (Uint i=0; i<coords.rows(); ++i)
//  for (Uint j=0; j<coords.cols(); ++j)
//  {
//    // multiply with 1e-5 (arbitrary) to avoid hash collisions
//    boost::hash_combine(seed,1e-3*coords(i,j));
//  }
//  return seed;
//}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
