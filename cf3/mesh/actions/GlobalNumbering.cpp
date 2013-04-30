// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

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
#include "common/List.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

#include "math/MatrixTypesConversion.hpp"
#include "math/Hilbert.hpp"
#include "math/Functions.hpp"
#include "math/Consts.hpp"

#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/BoundingBox.hpp"

#include "mesh/actions/GlobalNumbering.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;
  using namespace math::Functions;
  using namespace math::Consts;

  create_component_data_type( std::vector<boost::uint64_t> , mesh_actions_API , CVector_uint64 , "CVector<uint64>" );

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

  options().add("debug", m_debug)
      .description("Perform checks on validity")
      .pretty_name("Debug")
      .link_to(&m_debug);

  options().add("combined", true)
      .description("Combine nodes and elements in one global numbering")
      .pretty_name("Combined");
}

/////////////////////////////////////////////////////////////////////////////

void GlobalNumbering::execute()
{
  Mesh& mesh = *m_mesh;

  const Handle<BoundingBox>& global_bounding_box = mesh.global_bounding_box();

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
    cf3_assert(mesh.geometry_fields().size() > 0);
    for (Uint n=0; n<mesh.geometry_fields().size(); ++n)
    {
      cf3_assert(n<mesh.geometry_fields().rank().size());
      mesh.geometry_fields().rank()[n]=PE::Comm::instance().rank();
      cf3_assert(n<mesh.geometry_fields().glb_idx().size());
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
    mesh.raise_mesh_changed();
    return;
  }

  common::Table<Real>& coordinates = mesh.geometry_fields().coordinates();
  RealVector coord_vec(coordinates.row_size());

  if ( is_null( mesh.geometry_fields().get_child("hilbert_indices") ) )
    mesh.geometry_fields().create_component<CVector_uint64>("hilbert_indices");
  CVector_uint64& hilbert_indices = *Handle<CVector_uint64>(mesh.geometry_fields().get_child("hilbert_indices"));
  hilbert_indices.data().resize(coordinates.size());
  Uint i(0);
  boost_foreach(common::Table<Real>::ConstRow coords, coordinates.array() )
  {
    math::copy(coords,coord_vec);
    hilbert_indices.data()[i]=compute_glb_idx(coord_vec);

    if (m_debug)
      std::cout << "["<<PE::Comm::instance().rank() << "]  hashing node (local " << i << ")   (rank " << mesh.geometry_fields().rank()[i] << ")   coord("<< coord_vec.transpose() << ") to " << hilbert_indices.data()[i] << std::endl;
    ++i;
  }

  boost_foreach( Entities& elements, find_components_recursively<Entities>(mesh) )
  {
    RealMatrix element_coordinates(elements.element_type().nb_nodes(),coordinates.row_size());

    if ( is_null( elements.get_child("hilbert_indices") ) )
      elements.create_component<CVector_uint64>("hilbert_indices");
    CVector_uint64& hilbert_indices = *Handle<CVector_uint64>(elements.get_child("hilbert_indices"));
    hilbert_indices.data().resize(elements.size());

    for (Uint elem_idx=0; elem_idx<elements.size(); ++elem_idx)
    {
      elements.geometry_space().put_coordinates(element_coordinates,elem_idx);
      RealVector centroid(elements.element_type().dimension());
      elements.element_type().compute_centroid(element_coordinates,centroid);
      hilbert_indices.data()[elem_idx]=compute_glb_idx(centroid);
      if (m_debug)
        std::cout << "["<<PE::Comm::instance().rank() << "]  hashing elem "<< elements.uri().path() << "["<<elem_idx<<"] ("<<centroid.transpose()<<") to " << hilbert_indices.data()[elem_idx] << std::endl;

      //CFinfo << "hilbert_indices["<<elem_idx<<"] = " <<  hilbert_indices.data()[elem_idx] << CFendl;
    }
  }


  // In debug mode, check if no hashes are duplicated
  if (m_debug)
  {
    std::set<boost::uint64_t> hilbert_set;
    for (Uint i=0; i<hilbert_indices.data().size(); ++i)
    {
      if (hilbert_set.insert(hilbert_indices.data()[i]).second == false)  // it was already in the set
        throw ValueExists(FromHere(), "node "+to_str(i)+" ("+to_str(coordinates[i])+") is duplicated");
    }

    hilbert_set.clear();
    boost_foreach( Entities& elements, find_components_recursively<Entities>(mesh) )
    {
      CVector_uint64& hilbert_indices = *Handle<CVector_uint64>(elements.get_child("hilbert_indices"));
      for (Uint i=0; i<hilbert_indices.data().size(); ++i)
      {
        if (hilbert_set.insert(hilbert_indices.data()[i]).second == false)  // it was already in the set
          throw ValueExists(FromHere(), "elem "+elements.uri().path()+"["+to_str(i)+"] is duplicated");
      }
    }
    std::cout << "["<<PE::Comm::instance().rank() << "]  start renumbering" << std::endl;
  }


  // now renumber

  //------------------------------------------------------------------------------
  // create node_hilbert2loc mapping
  Dictionary& nodes = mesh.geometry_fields();
  std::map<boost::uint64_t,Uint> node_hilbert2loc;
  Uint loc_node_idx(0);
  boost_foreach(boost::uint64_t hilbert_idx, hilbert_indices.data())
    node_hilbert2loc[hilbert_idx]=loc_node_idx++;

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

  if (m_debug)
  {
    std::cout << "["<<PE::Comm::instance().rank() << "]  start_ids gathered" << std::endl;
  }


  //------------------------------------------------------------------------------
  // add glb_idx to owned nodes, broadcast/receive glb_idx for ghost nodes

  std::vector<boost::uint64_t> node_from(nb_owned_nodes);
  std::vector<boost::uint64_t> node_to(nb_owned_nodes);

  common::List<Uint>& nodes_glb_idx = mesh.geometry_fields().glb_idx();
  nodes_glb_idx.resize(nodes.size());

  Uint cnt=0;
  Uint glb_id = start_id_per_proc[PE::Comm::instance().rank()];
  for (Uint i=0; i<nodes.size(); ++i)
  {
    cf3_assert(nodes.rank()[i] < PE::Comm::instance().size());
    if ( ! nodes.is_ghost(i) )
    {
      nodes_glb_idx[i] = glb_id++;
      node_from[cnt] = hilbert_indices.data()[i];
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

    std::vector<boost::uint64_t> rcv_node_from(0);
    PE::Comm::instance().broadcast(node_from,rcv_node_from,root);
    std::vector<boost::uint64_t>        rcv_node_to(0);
    PE::Comm::instance().broadcast(node_to,rcv_node_to,root);

    if (m_debug)
    {
      std::cout << "["<<PE::Comm::instance().rank() << "]  received nodes from " << root << std::endl;
    }

    if (PE::Comm::instance().rank() != root)
    {
      for (Uint p=0; p<PE::Comm::instance().size(); ++p)
      {
        if (p == PE::Comm::instance().rank())
        {
          Uint rcv_idx(0);
          boost_foreach(const boost::uint64_t hilbert_idx, rcv_node_from)
          {
            if ( node_hilbert2loc.count(hilbert_idx) )
            {
              const Uint loc_idx = node_hilbert2loc[hilbert_idx];
              if (m_debug)
                std::cout << "["<<PE::Comm::instance().rank() << "]  will change node "<< hilbert_idx << " (local " << loc_idx<< ") to (global " << rcv_node_to[rcv_idx] << ")" << std::endl;
              cf3_assert(rcv_idx < rcv_node_to.size());
              nodes_glb_idx[loc_idx]=rcv_node_to[rcv_idx];
              cf3_assert(loc_idx < nodes_rank.size());
              cf3_assert_desc("node "+to_str(loc_idx)+ " with hilbert_idx " +to_str(hilbert_idx) +" must be a ghost, but is owned by "+to_str(nodes_rank[loc_idx]),nodes.is_ghost(loc_idx));
              nodes_rank[loc_idx]=std::min(root,nodes_rank[loc_idx]);
            }
            ++rcv_idx;
          }
          break;
        }
      }
    }
    if (m_debug)
    {
      std::cout << "["<<PE::Comm::instance().rank() << "]  changed nodes based on " << root << std::endl;
    }

  }

  if (m_debug)
  {
    std::cout << "["<<PE::Comm::instance().rank() << "]  checking node validity" << std::endl;
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
    std::vector<boost::uint64_t>& hilbert_indices = Handle<CVector_uint64>(elements.get_child("hilbert_indices"))->data();
    common::List<Uint>& elem_rank = elements.rank();
    elem_rank.resize(elements.size());

    //------------------------------------------------------------------------------
    // create elem_glb2loc mapping
    Uint nb_owned_elems=0;
    std::map<boost::uint64_t,Uint> elem_glb2loc;
    std::map<boost::uint64_t,Uint>::iterator elem_glb2loc_it;
    std::map<boost::uint64_t,Uint>::iterator hash_not_found = elem_glb2loc.end();

    for (Uint e=0; e<elements.size(); ++e)
    {
      elem_glb2loc [ hilbert_indices[e] ] = e;
      if ( ! elements.is_ghost(e) )
        ++nb_owned_elems;
    }


    std::vector<boost::uint64_t> send_hash(nb_owned_elems);
    std::vector<boost::uint64_t>   send_id(nb_owned_elems);

    common::List<Uint>& elements_glb_idx = elements.glb_idx();
    elements_glb_idx.resize(elements.size());
    cf3_assert(hilbert_indices.size() == elements.size());

    Uint cnt(0);
    for (Uint e=0; e<elements.size(); ++e)
    {

      if ( ! elements.is_ghost(e) )
      {
        if (m_debug)
          std::cout << "["<<PE::Comm::instance().rank() << "]  will change owned elem "<< hilbert_indices[e] << " (" << elements.uri().path() << "["<<e<<"]) to " << glb_id << std::endl;

        elements_glb_idx[e] = glb_id++;
        send_hash[cnt] = hilbert_indices[e];
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
      std::vector<boost::uint64_t> recv_hash(0);
      PE::Comm::instance().broadcast(send_hash,recv_hash,root);
      std::vector<boost::uint64_t>        recv_id(0);
      PE::Comm::instance().broadcast(send_id,recv_id,root);
      if (PE::Comm::instance().rank() != root)
      {
        for (Uint p=0; p<PE::Comm::instance().size(); ++p)
        {
          if (p == PE::Comm::instance().rank())
          {
            Uint recv_idx(0);
            boost_foreach(const boost::uint64_t hash, recv_hash)
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
    std::set<boost::uint64_t> hilbert_set;
    for (Uint i=0; i<nodes_glb_idx.size(); ++i)
    {
      if (hilbert_set.insert(nodes_glb_idx[i]).second == false)  // it was already in the set
        throw ValueExists(FromHere(), "node "+to_str(i)+" is duplicated");
      if (nodes_glb_idx[i] == uint_max())
        throw BadValue(FromHere(), "node " + to_str(i)+" doesn't have glb_idx");
    }

    boost_foreach( Entities& elements, find_components_recursively<Entities>(mesh) )
    {
      common::List<Uint>& elements_glb_idx = elements.glb_idx();
      for (Uint i=0; i<elements.size(); ++i)
      {
        if (hilbert_set.insert(elements_glb_idx[i]).second == false)  // it was already in the set
          throw ValueExists(FromHere(), "elem "+elements.uri().path()+"["+to_str(i)+"] is duplicated");
        if (elements_glb_idx[i] == uint_max())
          throw BadValue(FromHere(), "elem "+elements.uri().path()+"["+to_str(i)+"] doesn't have glb_idx");

      }
    }
  }


  mesh.geometry_fields().remove_component(hilbert_indices);

  boost_foreach( Entities& elements, find_components_recursively<Entities>(mesh) )
  {
    elements.remove_component("hilbert_indices");
  }

  mesh.raise_mesh_changed();
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
