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
#include "common/Tags.hpp"
#include "common/DynTable.hpp"
#include "common/List.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/CommPattern.hpp"
#include "common/PE/debug.hpp"

#include "math/BoundingBox.hpp"
#include "math/Hilbert.hpp"

#include "mesh/Field.hpp"
#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/Region.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"


#include "math/Consts.hpp"
#define UNKNOWN math::Consts::uint_max()

namespace cf3 {
namespace mesh {

using namespace boost::assign;

using namespace common;
using namespace common::PE;

common::ComponentBuilder < DiscontinuousDictionary, Component, LibMesh >  DiscontinuousDictionary_Builder;

////////////////////////////////////////////////////////////////////////////////

namespace detail
{
  /// Helper to compute the centroid
  struct ComputeCentroid
  {
    ComputeCentroid(const Entities& entities) :
      m_entities(entities),
      centroid(entities.element_type().dimension()),
      normal(entities.element_type().dimension()),
      elem_coords(entities.geometry_space().shape_function().nb_nodes(),entities.element_type().dimension())
    {
    }

    /// Compute the centroid, adapting using the normal so that internal boundaries that exist with both orientations
    /// get a distinct centroid
    const RealVector& operator()(const Uint element_index)
    {
      m_entities.geometry_space().put_coordinates(elem_coords,element_index);
      m_entities.element_type().compute_centroid(elem_coords, centroid);
      if(m_entities.element_type().dimension() != m_entities.element_type().dimensionality())
      {
        const Real tol = 0.0001;
        try
        {
          m_entities.element_type().compute_normal(elem_coords, normal);
        }
        catch(NotImplemented& e)
        {
          normal.setZero();
          CFdebug << e.what() << ", ignoring normal correction to face centroid" << CFendl;
        }
        centroid += tol*normal;
      }

      return centroid;
    }

    const Entities& m_entities;

    RealVector centroid;
    RealVector normal;
    RealMatrix elem_coords;
  };

  template <typename T>
  void DiscontinuousDictionary_send_receive(const Uint send_to_pid, std::vector<T>& send, const Uint receive_from_pid, std::vector<T>& receive)
  {
    if (send_to_pid == PE::Comm::instance().rank() &&
        receive_from_pid == PE::Comm::instance().rank())
    {
      receive = send;
        return;
    }
    size_t recv_size;
    size_t send_size = send.size();
    MPI_Sendrecv(&send_size, 1, PE::get_mpi_datatype<size_t>(), (int)send_to_pid, 0,
                 &recv_size, 1, PE::get_mpi_datatype<size_t>(), (int)receive_from_pid, 0,
                 PE::Comm::instance().communicator(), MPI_STATUS_IGNORE);
    receive.resize(recv_size);
    MPI_Sendrecv(&send[0], (int)send.size(), PE::get_mpi_datatype<T>(), (int)send_to_pid, 0,
                 &receive[0], (int)receive.size(), PE::get_mpi_datatype<T>(), (int)receive_from_pid, 0,
                 PE::Comm::instance().communicator(), MPI_STATUS_IGNORE);
  }
}

DiscontinuousDictionary::DiscontinuousDictionary ( const std::string& name  ) :
  Dictionary( name )
{
  m_is_continuous = false;
}

////////////////////////////////////////////////////////////////////////////////

DiscontinuousDictionary::~DiscontinuousDictionary()
{
}

////////////////////////////////////////////////////////////////////////////////

void DiscontinuousDictionary::rebuild_spaces_from_geometry()
{
  // STEP 1: Assign the space connectivity table in the topology
  // -----------------------------------------------------------
  Uint field_idx = 0;
  boost_foreach(const Handle<Entities>& entities, entities_range())
  {
    const Space& entities_space = *space(entities);
    Connectivity& space_connectivity = const_cast<Connectivity&>(entities_space.connectivity());
    space_connectivity.resize(entities->size());
    Uint nb_nodes_per_elem = entities_space.shape_function().nb_nodes();
    const Uint nb_elems = entities->size();
    for (Uint elem=0; elem<nb_elems; ++elem)
    {
      for (Uint node=0; node<nb_nodes_per_elem; ++node)
        space_connectivity[elem][node] = field_idx++;
    }
  }

  // STEP 2: Resize this space
  // -------------------------
  resize(field_idx);

  // STEP 3: fix unknown glb_idx of nodes
  // ------------------------------------
  //  (1) Count the number of owned entries per process (owned when element it belongs to is owned)
  //  (2) glb_idx for owned nodes is filled in
  //  (3) Create map< glb_idx , element > of all elements. It will be used to match different cpu-elems
  //  (4) ghost elements entries are communicated for lookup
  //  (5) glb_idx of ghost entries are received back

  // (1)

  Uint nb_owned(0);
  std::vector<Uint> nb_ghosts_belonging_to_rank(PE::Comm::instance().size());
  std::map<boost::uint64_t, Entity > glb_elem_to_loc;
  boost_foreach(const Handle<Entities>& entities_handle, entities_range())
  {
    Entities& entities = *entities_handle;
    const Connectivity& space_connectivity = space(entities).connectivity();
    Uint nb_states_per_cell = space(entities).shape_function().nb_nodes();

    for (Uint e=0; e<entities.size(); ++e)
    {
      glb_elem_to_loc[ entities.glb_idx()[e] ] = Entity(entities,e);
      if (entities.rank()[e] == PE::Comm::instance().rank())
      {
        nb_owned+=nb_states_per_cell;
      }
      else
      {
        ++nb_ghosts_belonging_to_rank[ entities.rank()[e] ];
      }
      boost_foreach(const Uint idx, space_connectivity[e])
      {
        rank()[idx] = entities.rank()[e];
        glb_idx()[idx] = entities.glb_idx()[e]; // temporarily store element-glb-idx in node-glb-idx
      }
    }
  }

  std::vector<Uint> nb_owned_per_proc(Comm::instance().size(),nb_owned);
  if (Comm::instance().is_active())
    Comm::instance().all_gather(nb_owned, nb_owned_per_proc);

  std::vector<Uint> start_id_per_proc(Comm::instance().size(),0);
  for (Uint i=0; i<Comm::instance().size(); ++i)
  {
    start_id_per_proc[i] = (i==0? 0 : start_id_per_proc[i-1]+nb_owned_per_proc[i-1]);
  }

  // (2)
  Uint id = start_id_per_proc[Comm::instance().rank()];
  boost_foreach(const Handle<Entities>& entities_handle, entities_range())
  {
    Entities& entities = *entities_handle;
    const Connectivity& space_connectivity = space(entities).connectivity();
    for (Uint e=0; e<entities.size(); ++e)
    {
      boost_foreach(const Uint idx, space_connectivity[e])
      {
        if (rank()[idx] == PE::Comm::instance().rank() || PE::Comm::instance().size() == 1)
        {
          glb_idx()[idx] = id++;
        }
      }
    }
  }

  // (3)
  std::vector< std::vector<boost::uint64_t> > ghosts_elem_idx(PE::Comm::instance().size());
  for(Uint p=0; p<PE::Comm::instance().size(); ++p)
    ghosts_elem_idx[p].reserve( nb_ghosts_belonging_to_rank[p] );


  if (PE::Comm::instance().size() > 1)
  {
    boost_foreach(const Handle<Entities>& entities_handle, entities_range())
    {
      Entities& entities = *entities_handle;
      const Space& entities_space = space(entities);
      Uint nb_states_per_elem = entities_space.shape_function().nb_nodes();

      for (Uint e=0; e<entities.size(); ++e)
      {
        if (entities.is_ghost(e)) // if is ghost
        {
          cf3_always_assert(entities.rank()[e] != UNKNOWN);
          ghosts_elem_idx[entities.rank()[e]].push_back(entities.glb_idx()[e]);
        }
      }
    }

    // (5) Search if this process contains the unknown ghosts of other processes
    std::vector< std::vector<boost::uint64_t> > received_glb_elem_node_indices(PE::Comm::instance().size());

    for (Uint pid=0; pid<Comm::instance().size(); ++pid)
    {
      const Uint pid_send = (PE::Comm::instance().rank() + pid) %
                            PE::Comm::instance().size();
      const Uint pid_recv = (PE::Comm::instance().size() + PE::Comm::instance().rank() - pid) %
                            PE::Comm::instance().size();

      std::vector<boost::uint64_t>& send_glb_ghost_elem_indices = ghosts_elem_idx[pid_send];
      std::vector<boost::uint64_t> received_glb_ghost_elem_indices;
      detail::DiscontinuousDictionary_send_receive( pid_send, send_glb_ghost_elem_indices,
                                                    pid_recv, received_glb_ghost_elem_indices );

      std::vector<boost::uint64_t> send_glb_elem_node_indices;
      send_glb_elem_node_indices.reserve(received_glb_ghost_elem_indices.size());

      boost_foreach( const boost::uint64_t& recv_glb_idx, received_glb_ghost_elem_indices )
      {
        cf3_assert( glb_elem_to_loc.count( recv_glb_idx ) > 0);
        const Entity& found_entity = glb_elem_to_loc[recv_glb_idx];
        const Connectivity& space_connectivity = space(*found_entity.comp).connectivity();
        send_glb_elem_node_indices.push_back( glb_idx()[space_connectivity[found_entity.idx][0]] );
      }

      const Uint pid_send_glb_elem_node_indices = pid_recv;
      const Uint pid_recv_glb_elem_node_indices = pid_send;

      detail::DiscontinuousDictionary_send_receive( pid_send_glb_elem_node_indices, send_glb_elem_node_indices,
                                                    pid_recv_glb_elem_node_indices, received_glb_elem_node_indices[pid_recv_glb_elem_node_indices] );
    }

    std::vector<Uint> count(PE::Comm::instance().size(), 0);
    boost_foreach(const Handle<Entities>& entities_handle, entities_range())
    {
      Entities& entities = *entities_handle;
      const Space& entities_space = space(entities);
      const Connectivity& space_connectivity = space(entities).connectivity();
      Uint nb_states_per_elem = entities_space.shape_function().nb_nodes();
      for (Uint e=0; e<entities.size(); ++e)
      {
        if (entities.is_ghost(e) && PE::Comm::instance().size() > 1) // if is ghost
        {
          const Uint p = entities.rank()[e];
          const Uint start_id = received_glb_elem_node_indices[entities.rank()[e]][count[p]];
          for (Uint n=0; n<nb_states_per_elem; ++n)
          {
            glb_idx()[space_connectivity[e][n]] = start_id + n;
          }
          ++count[p];
        }
      }
    }
  }
  create_coordinates();
}

////////////////////////////////////////////////////////////////////////////////

void DiscontinuousDictionary::rebuild_node_to_element_connectivity()
{
  // Reserve memory in m_connectivity->array()
  m_connectivity->array().resize(size());
  for (Uint n=0; n<size(); ++n)
  {
    m_connectivity->set_row_size(n,1);
  }
  boost_foreach (const Handle<Space>& space, spaces())
  {
    for (Uint elem_idx=0; elem_idx<space->size(); ++elem_idx)
    {
      boost_foreach (const Uint node_idx, space->connectivity()[elem_idx])
      {
        m_connectivity->array()[node_idx][0]=SpaceElem(*space,elem_idx);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

#undef UNKNOWN
