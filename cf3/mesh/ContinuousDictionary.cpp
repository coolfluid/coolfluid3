// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>

#include "common/Log.hpp"
#include "common/PropertyList.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/EventHandler.hpp"
#include "common/StringConversion.hpp"
#include "common/Tags.hpp"
#include "common/DynTable.hpp"
#include "common/List.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/CommPattern.hpp"

#include "mesh/ContinuousDictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/ContinuousDictionary.hpp"
#include "mesh/Region.hpp"
#include "mesh/Mesh.hpp"

#include "mesh/Cells.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

#include "math/Consts.hpp"
#include "math/MatrixTypesConversion.hpp"
#include "math/Hilbert.hpp"
#include "math/BoundingBox.hpp"
#define UNKNOWN math::Consts::uint_max()

namespace cf3 {
namespace mesh {

using namespace boost::assign;

using namespace common;
using namespace common::PE;

common::ComponentBuilder < ContinuousDictionary, Component, LibMesh >  ContinuousDictionary_Builder;

////////////////////////////////////////////////////////////////////////////////

ContinuousDictionary::ContinuousDictionary ( const std::string& name  ) :
  Dictionary( name )
{
}

////////////////////////////////////////////////////////////////////////////////

ContinuousDictionary::~ContinuousDictionary()
{
}

////////////////////////////////////////////////////////////////////////////////

void ContinuousDictionary::rebuild_spaces_from_geometry()
{
  std::set<boost::uint64_t> points;
  RealMatrix elem_coordinates;
  Uint dim = DIM_0D;


  // step 1: collect nodes in a set
  // ------------------------------

  // (a) Create bounding box of all coordinates, to define a space
  //     This bounding box is not the one from the mesh, as it doesn't have
  //     to cover the mesh, or can exist in more than one mesh.
  math::BoundingBox bounding_box;
  boost_foreach(const Handle<Entities>& entities_handle, entities_range())
  {
    Entities& entities = *entities_handle;
    const ShapeFunction& shape_function = space(entities).shape_function();
    RealVector node_coord(entities.element_type().dimension());
    for (Uint elem=0; elem<entities.size(); ++elem)
    {
      elem_coordinates = entities.geometry_space().get_coordinates(elem);
      for (Uint node=0; node<elem_coordinates.rows(); ++node)
      {
        node_coord = elem_coordinates.row(node);
        bounding_box.extend(node_coord);
      }
    }
  }
  bounding_box.make_global();

  // (b) Create unique indices for every coordinate
  math::Hilbert compute_glb_idx(bounding_box, 20);  // functor
  boost_foreach(const Handle<Entities>& entities_handle, entities_range())
  {
    Entities& entities = *entities_handle;
    const ShapeFunction& shape_function = space(entities).shape_function();
    for (Uint elem=0; elem<entities.size(); ++elem)
    {
      elem_coordinates = entities.geometry_space().get_coordinates(elem);
      for (Uint node=0; node<shape_function.nb_nodes(); ++node)
      {
        RealVector space_coordinates = entities.element_type().shape_function().value(shape_function.local_coordinates().row(node)) * elem_coordinates ;
        boost::uint64_t hash = compute_glb_idx(space_coordinates);
        points.insert( hash );
      }
    }
  }

  // (c) Create the coordinates field
  Field& coordinates = create_field(mesh::Tags::coordinates(),"coords[vector]");

  // step 3: resize
  // --------------
  resize(points.size());
  m_coordinates = coordinates.handle<Field>();
  for (Uint i=0; i<size(); ++i)
    rank()[i] = UNKNOWN;

  // step 2: collect nodes in a set
  // ------------------------------
  boost_foreach(const Handle<Entities>& entities_handle, entities_range())
  {
    Entities& entities = *entities_handle;
    Dictionary& geometry = entities.geometry_fields();
    Connectivity& geometry_node_connectivity = entities.geometry_space().connectivity();
    common::List<Uint>& geometry_rank = entities.geometry_fields().rank();
    const ShapeFunction& shape_function = space(entities).shape_function();
    Connectivity& connectivity = const_cast<Space&>(space(entities)).connectivity();
    connectivity.resize(entities.size());
    for (Uint elem=0; elem<entities.size(); ++elem)
    {
      elem_coordinates = entities.geometry_space().get_coordinates(elem);
      for (Uint node=0; node<shape_function.nb_nodes(); ++node)
      {
        RealVector space_coordinates = entities.element_type().shape_function().value(shape_function.local_coordinates().row(node)) * elem_coordinates ;
        boost::uint64_t hash = compute_glb_idx(space_coordinates);
        Uint idx = std::distance(points.begin(), points.find(hash));
        connectivity[elem][node] = idx;
        coordinates.set_row(idx, space_coordinates);
        rank()[idx] = UNKNOWN;
        glb_idx()[idx] = UNKNOWN;
      }
    }
  }

  // step 5: fix unknown ranks
  // -------------------------
  cf3_assert(size() == m_rank->size());
  cf3_assert(size() == m_glb_idx->size());
  cf3_assert(size() == m_coordinates->size());

  boost_foreach(const Handle<Entities>& entities_handle, entities_range())
  {
    Entities& entities = *entities_handle;
    const Connectivity& space_connectivity = space(entities).connectivity();
    for (Uint e=0; e<entities.size(); ++e)
    {
      boost_foreach(const Uint node, space_connectivity[e])
          rank()[node] = std::min(rank()[node] , entities.rank()[e] );
    }
  }

  std::map<boost::uint64_t,Uint> hash_to_idx;
  std::map<boost::uint64_t,Uint>::iterator hash_to_idx_iter;
  std::map<boost::uint64_t,Uint>::iterator hash_not_found = hash_to_idx.end();

  std::vector<boost::uint64_t> coord_hash(size());
  RealVector dummy(coordinates.row_size());
  Uint c(0);
  for (Uint i=0; i<size(); ++i)
  {
    math::copy(coordinates[i],dummy);
    boost::uint64_t hash = compute_glb_idx(dummy);
    hash_to_idx[hash] = i;
    coord_hash[c++]=hash;
  }

  // - Communicate unknown ranks to all processes
  std::vector< std::vector<boost::uint64_t> > recv_hash(Comm::instance().size());
  if (Comm::instance().is_active())
    Comm::instance().all_gather(coord_hash,recv_hash);
  else
    recv_hash[0] = coord_hash;

  // - Search this process contains the missing ranks of other processes
  std::vector< std::vector<Uint> > send_found_on_rank(Comm::instance().size());
  for (Uint p=0; p<Comm::instance().size(); ++p)
  {
    send_found_on_rank[p].resize(recv_hash[p].size(),UNKNOWN);
    {
      for (Uint h=0; h<recv_hash[p].size(); ++h)
      {
        hash_to_idx_iter = hash_to_idx.find(recv_hash[p][h]);
        if ( hash_to_idx_iter != hash_not_found )
        {
          send_found_on_rank[p][h] = rank()[ hash_to_idx_iter->second ];
        }
      }
    }

  }

  // - Communicate which processes found the missing ranks
  std::vector< std::vector<Uint> > recv_found_on_rank(Comm::instance().size());
  if (Comm::instance().is_active())
    Comm::instance().all_to_all(send_found_on_rank,recv_found_on_rank);
  else
    recv_found_on_rank[0] = send_found_on_rank[0];


  // - Set the missing rank to the lowest process number that found it
  for (Uint n=0; n<size(); ++n)
  {
    Uint rank_that_owns = UNKNOWN;

    for (Uint p=0; p<Comm::instance().size(); ++p)
    {
      rank_that_owns = std::min(recv_found_on_rank[p][n], rank_that_owns);
    }
    rank()[n] = rank_that_owns;

    cf3_assert(rank()[n] != UNKNOWN);
  }

  // step 5: fix unknown glb_idx
  // ---------------------------
  Uint nb_owned = 0;
  std::vector<Uint> ghosts; ghosts.reserve(size());
  for (Uint i=0; i<size(); ++i)
  {
    if (is_ghost(i))
      ghosts.push_back(i);
    else
      ++nb_owned;
  }
  std::vector<boost::uint64_t> ghosts_hashed(ghosts.size());
  for (Uint g=0; g<ghosts.size(); ++g)
  {
    math::copy(coordinates[ghosts[g]], dummy);
    ghosts_hashed[g] = compute_glb_idx(dummy);
  }
  std::vector<Uint> nb_owned_per_proc(Comm::instance().size(),nb_owned);
  if( Comm::instance().is_active() )
    Comm::instance().all_gather(nb_owned, nb_owned_per_proc);

  std::vector<Uint> start_id_per_proc(Comm::instance().size());

  Uint start_id=0;
  for (Uint p=0; p<Comm::instance().size(); ++p)
  {
    start_id_per_proc[p] = start_id;
    start_id += nb_owned_per_proc[p];
  }
  start_id = start_id_per_proc[Comm::instance().rank()];
  for (Uint i=0; i<size(); ++i)
  {
    if (! is_ghost(i))
      glb_idx()[i] = start_id++;
    else
      glb_idx()[i] = UNKNOWN;
  }

  std::vector< std::vector<boost::uint64_t> > recv_ghosts_hashed(Comm::instance().size());
  if (Comm::instance().is_active())
    Comm::instance().all_gather(ghosts_hashed,recv_ghosts_hashed);
  else
    recv_ghosts_hashed[0] = ghosts_hashed;

  // - Search this process contains the missing ranks of other processes
  std::vector< std::vector<Uint> > send_glb_idx_on_rank(Comm::instance().size());
  for (Uint p=0; p<Comm::instance().size(); ++p)
  {
    send_glb_idx_on_rank[p].resize(recv_ghosts_hashed[p].size(),UNKNOWN);
    if (p!=Comm::instance().rank())
    {
      for (Uint h=0; h<recv_ghosts_hashed[p].size(); ++h)
      {
        hash_to_idx_iter = hash_to_idx.find(recv_ghosts_hashed[p][h]);
        if ( hash_to_idx_iter != hash_not_found )
        {
          send_glb_idx_on_rank[p][h] = glb_idx()[hash_to_idx_iter->second];
        }
      }
    }
  }

  // - Communicate which processes found the missing ghosts
  std::vector< std::vector<Uint> > recv_glb_idx_on_rank(Comm::instance().size());
  if (Comm::instance().is_active())
    Comm::instance().all_to_all(send_glb_idx_on_rank,recv_glb_idx_on_rank);
  else
    recv_glb_idx_on_rank[0] = send_glb_idx_on_rank[0];

  // - Set the missing rank to the lowest process number that found it
  for (Uint g=0; g<ghosts.size(); ++g)
  {
    cf3_assert(rank()[ghosts[g]] < Comm::instance().size());
    glb_idx()[ghosts[g]] = recv_glb_idx_on_rank[rank()[ghosts[g]]][g];
  }
}

////////////////////////////////////////////////////////////////////////////////

void ContinuousDictionary::rebuild_node_to_element_connectivity()
{
  // Reserve memory in m_connectivity->array()
  m_connectivity->array().resize(size());

  std::vector<Uint> connectivity_sizes(size());
  boost_foreach (const Handle<Space>& space, spaces() )
  {
    for (Uint elem_idx=0; elem_idx<space->size(); ++elem_idx)
    {
      boost_foreach (const Uint node_idx, space->connectivity()[elem_idx])
      {
        cf3_assert_desc(to_str(node_idx)+"<"+to_str(size())+" --> something wrong with the element-node connectivity table",node_idx<size());
        ++connectivity_sizes[node_idx];
      }
    }
  }
  for (Uint i=0; i<size(); ++i)
  {
    m_connectivity->array()[i].clear();
    m_connectivity->array()[i].reserve(connectivity_sizes[i]);
  }

  boost_foreach (const Handle<Space>& space, spaces())
  {
    for (Uint elem_idx=0; elem_idx<space->size(); ++elem_idx)
    {
      boost_foreach (const Uint node_idx, space->connectivity()[elem_idx])
      {
        m_connectivity->array()[node_idx].push_back(SpaceElem(*space,elem_idx));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

#undef UNKNOWN
