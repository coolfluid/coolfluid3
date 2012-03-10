// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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
#include "common/PE/Comm.hpp"
#include "common/PE/CommPattern.hpp"

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

//boost::uint64_t DiscontinuousDictionary::hash_value(const RealMatrix& coords)
//{
//  boost::uint64_t seed=0;
//  for (Uint i=0; i<coords.rows(); ++i)
//  for (Uint j=0; j<coords.cols(); ++j)
//  {
//    // multiply with 1e-5 (arbitrary) to avoid hash collisions
//    boost::hash_combine(seed,1e-6*coords(i,j));
//  }
//  return seed;
//}


void DiscontinuousDictionary::create_connectivity_in_space()
{
  math::BoundingBox bounding_box;

  // STEP 1: Assign the space connectivity table in the topology
  // -----------------------------------------------------------
  Uint field_idx = 0;
  boost_foreach(const Handle<Entities>& entities, entities_range())
  {
    const Space& entities_space = *space(entities);
    Connectivity& space_connectivity = const_cast<Connectivity&>(entities_space.connectivity());
    space_connectivity.resize(entities->size());
    RealMatrix elem_coords(entities->geometry_space().shape_function().nb_nodes(),entities->element_type().dimension());
    RealVector centroid(entities->element_type().dimension());

    Uint nb_nodes_per_elem = entities_space.shape_function().nb_nodes();
    const Uint nb_elems = entities->size();
    for (Uint elem=0; elem<nb_elems; ++elem)
    {
      entities->geometry_space().put_coordinates(elem_coords,elem);
      entities->element_type().compute_centroid(elem_coords,centroid);
      bounding_box.extend(centroid);

      for (Uint node=0; node<nb_nodes_per_elem; ++node)
        space_connectivity[elem][node] = field_idx++;
    }
  }
  bounding_box.make_global();

  // STEP 2: Resize this space
  // -------------------------
  resize(field_idx);

  // STEP 3: fix the unknown ranks of this space
  // -------------------------------------------
  //  - The rank of every node will be the rank of the element it belongs to
  boost_foreach(const Handle<Entities>& entities_handle, entities_range())
  {
    Entities& entities = *entities_handle;
    cf3_assert_desc("mesh not properly constructed",entities.rank().size() == entities.size());
    const Connectivity& space_connectivity = space(entities).connectivity();
    for (Uint e=0; e<entities.size(); ++e)
    {
      boost_foreach(const Uint idx, space_connectivity[e])
      {
        // cf3_assert(entities.rank()[e] < PE::Comm::instance().size());
        rank()[idx] = entities.rank()[e];
      }
    }
  }

  // (3)
  std::map<boost::uint64_t,Entity> hash_to_elements;
  std::deque<Entity> unknown_rank_elements;
  std::deque<boost::uint64_t> unknown_rank_elements_hash_deque;

  math::Hilbert compute_glb_idx(bounding_box,20);

  boost_foreach(const Handle<Entities>& entities_handle, entities_range())
  {
    Entities& entities = *entities_handle;
    const Space& entities_space = space(entities);
    Uint nb_states_per_elem = entities_space.shape_function().nb_nodes();
    RealMatrix elem_coords(entities.geometry_space().shape_function().nb_nodes(),entities.element_type().dimension());
    RealVector centroid(entities.element_type().dimension());
    for (Uint e=0; e<entities.size(); ++e)
    {
      entities.geometry_space().put_coordinates(elem_coords,e);
      entities.element_type().compute_centroid(elem_coords,centroid);
      boost::uint64_t hash = compute_glb_idx(centroid);
//      std::cout << "["<<PE::Comm::instance().rank() << "]  hashed "<< entities.uri().path() << "["<<e<<"]) to " << hash << std::endl;
      bool inserted = hash_to_elements.insert( std::make_pair(hash, Entity(entities,e)) ).second;
      if (! inserted)
      {
        std::stringstream msg;
        msg <<"Duplicate hash " << hash << " detected for element " << entities.uri() << " with centroid (" << centroid.transpose() << ")\n";
        throw ValueExists(FromHere(), msg.str());
      }
      if(entities.rank()[e] == UNKNOWN)
      {
        unknown_rank_elements.push_back(Entity(entities,e));
        unknown_rank_elements_hash_deque.push_back(hash);
      }
      //        std::cout << std::endl;
    }
  }

  // copy deque in vector, delete deque
  std::vector<boost::uint64_t> unknown_rank_elements_hashed(unknown_rank_elements_hash_deque.size());
  for (Uint g=0; g<unknown_rank_elements_hash_deque.size(); ++g)
  {
    unknown_rank_elements_hashed[g] = unknown_rank_elements_hash_deque[g];
  }
  unknown_rank_elements_hash_deque.clear();


  // (4)
  std::vector< std::vector<boost::uint64_t> > recv_unknown_rank_elements_hashed(Comm::instance().size());
  if (Comm::instance().is_active())
    Comm::instance().all_gather(unknown_rank_elements_hashed,recv_unknown_rank_elements_hashed);
  else
    recv_unknown_rank_elements_hashed[0] = unknown_rank_elements_hashed;

  // (5) Search if this process contains the elements with unknown ranks of other processes
  std::map<boost::uint64_t,Entity>::iterator hash_to_elements_iter;
  std::map<boost::uint64_t,Entity>::iterator hash_not_found = hash_to_elements.end();
  std::vector< std::vector<Uint> > send_found_on_rank(Comm::instance().size());
  for (Uint p=0; p<Comm::instance().size(); ++p)
  {
    send_found_on_rank[p].resize(recv_unknown_rank_elements_hashed[p].size(),UNKNOWN);
    if (p!=Comm::instance().rank())
    {
      for (Uint h=0; h<recv_unknown_rank_elements_hashed[p].size(); ++h)
      {
        //          std::cout << PERank << "looking for hash " << recv_unknown_rank_elements_hashed[p][h] << " for proc " << p ;
        hash_to_elements_iter = hash_to_elements.find(recv_unknown_rank_elements_hashed[p][h]);
        if ( hash_to_elements_iter != hash_not_found )
        {
          const Space& entities_space = space(*hash_to_elements_iter->second.comp);
          const Uint elem_idx = hash_to_elements_iter->second.idx;
          if (rank()[entities_space.connectivity()[elem_idx][0]] == PE::Comm::instance().rank())
          {
            send_found_on_rank[p][h] = PE::Comm::instance().rank();
            //              std::cout << " ---> found " ;
          }
          else
          {
            send_found_on_rank[p][h] = UNKNOWN-1;
            //              std::cout << " ---> found but unknown" ;
          }
        }
        //          std::cout << std::endl;
      }
    }
    else
    {
      for (Uint h=0; h<recv_unknown_rank_elements_hashed[p].size(); ++h)
        send_found_on_rank[p][h] = UNKNOWN-1;
    }
  }

  // (6)
  std::vector< std::vector<Uint> > recv_found_on_rank(Comm::instance().size());
  if (Comm::instance().is_active())
    Comm::instance().all_to_all(send_found_on_rank,recv_found_on_rank);
  else
    recv_found_on_rank[0] = send_found_on_rank[0];

  for (Uint g=0; g<unknown_rank_elements.size(); ++g)
  {
    const Space& entities_space = space(*unknown_rank_elements[g].comp);
    const Uint elem_idx = unknown_rank_elements[g].idx;
    cf3_assert(elem_idx<entities_space.connectivity().size());
    cf3_assert(elem_idx<entities_space.support().rank().size());

    const Uint first_loc_idx = entities_space.connectivity()[elem_idx][0];
    Uint recv_rank = UNKNOWN;
    Uint found_rank = UNKNOWN;
    //      std::cout << PERank << "checking hash " << unknown_rank_elements_hashed[g] << ": ";
    for (Uint p=0; p<Comm::instance().size(); ++p)
    {
      //        std::cout << recv_found_on_rank[p][g] << "   ";
      if ( recv_found_on_rank[p][g] < recv_rank )
      {
        recv_rank = recv_found_on_rank[p][g];
        found_rank = p;
      }
    }
    //      std::cout << std::endl;
    if (found_rank == UNKNOWN) // ---> Nobody owns this. This is because it is notfound in
      found_rank = PE::Comm::instance().rank();
    //        throw ValueNotFound(FromHere(), "Could not find rank for element "+entities_space.uri().path()+"["+to_str(elem_idx)+"] with hash "+to_str(unknown_rank_elements_hashed[g]));

    //      std::cout << PERank << "set unknown element rank: " << unknown_rank_elements_hashed[g] << " --> " << found_rank << std::endl;

    for (Uint s=0; s<entities_space.shape_function().nb_nodes(); ++s)
    {
      cf3_assert(first_loc_idx+s<rank().size());
      rank()[first_loc_idx+s] = found_rank;
    }
  }


  // STEP 4: fix unknown glb_idx
  // ---------------------------
  //  (1) Count the number of owned entries per process (owned when element it belongs to is owned)
  //  (2) glb_idx is filled in, ghost-entries are marked by a value "UNKNOWN" (=uint_max)
  //  (3) Create map< hash-value , element > of all elements. It will be used to match different cpu-elems
  //  (4) hash-values of ghost elements entries are communicated for lookup
  //  (5) lookup of received hash-values from other processes are translated into owned glb_idx of space-entries
  //  (6) glb_idx of ghost entries are received back


  // (1)

  Uint nb_owned(0);
  boost_foreach(const Handle<Entities>& entities_handle, entities_range())
  {
    Entities& entities = *entities_handle;
    Uint nb_states_per_cell = space(entities).shape_function().nb_nodes();

    for (Uint e=0; e<entities.size(); ++e)
    {
      if (rank()[space(entities_handle)->connectivity()[e][0]] == PE::Comm::instance().rank())
        nb_owned+=nb_states_per_cell;
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
      if (rank()[space_connectivity[e][0]] == PE::Comm::instance().rank())
      {
        boost_foreach(const Uint idx, space_connectivity[e])
            glb_idx()[idx] = id++;
      }
      else
      {
        boost_foreach(const Uint idx, space_connectivity[e])
            glb_idx()[idx] = UNKNOWN;
      }
    }
  }

  // (3)
  std::deque<Entity> ghosts;
  std::deque<boost::uint64_t> ghosts_hashed_deque;

  boost_foreach(const Handle<Entities>& entities_handle, entities_range())
  {
    Entities& entities = *entities_handle;
    const Space& entities_space = space(entities);
    Uint nb_states_per_elem = entities_space.shape_function().nb_nodes();
    RealMatrix elem_coords(entities.geometry_space().shape_function().nb_nodes(),entities.element_type().dimension());
    RealVector centroid(entities.element_type().dimension());
    for (Uint e=0; e<entities.size(); ++e)
    {
      entities.geometry_space().put_coordinates(elem_coords,e);
      entities.element_type().compute_centroid(elem_coords,centroid);
      boost::uint64_t hash = compute_glb_idx(centroid);
      //        std::cout << "["<<PE::Comm::instance().rank() << "]  hashed "<< entities.uri().path() << "["<<e<<"]) to " << hash;
      if (rank()[entities_space.connectivity()[e][0]] != PE::Comm::instance().rank()) // if is ghost
      {
        cf3_assert(rank()[entities_space.connectivity()[e][0]] != UNKNOWN);
        //          std::cout << "    --> is ghost, owned by " << entities.rank()[e] ;
        ghosts.push_back(Entity(entities,e));
        ghosts_hashed_deque.push_back(hash);
      }
      //        std::cout << std::endl;
    }
  }

  // copy deque in vector, delete deque
  std::vector<boost::uint64_t> ghosts_hashed(ghosts.size());
  for (Uint g=0; g<ghosts.size(); ++g)
  {
    ghosts_hashed[g] = ghosts_hashed_deque[g];
  }
  ghosts_hashed_deque.clear();

  // (4)
  std::vector< std::vector<boost::uint64_t> > recv_ghosts_hashed(Comm::instance().size());
  if (Comm::instance().is_active())
    Comm::instance().all_gather(ghosts_hashed,recv_ghosts_hashed);
  else
    recv_ghosts_hashed[0] = ghosts_hashed;

  // (5) Search if this process contains the unknown ghosts of other processes
  std::vector< std::vector<Uint> > send_glb_idx_on_rank(Comm::instance().size());
  for (Uint p=0; p<Comm::instance().size(); ++p)
  {
    send_glb_idx_on_rank[p].resize(recv_ghosts_hashed[p].size(),UNKNOWN);
    if (p!=Comm::instance().rank())
    {
      for (Uint h=0; h<recv_ghosts_hashed[p].size(); ++h)
      {
        //          std::cout << PERank << "looking for hash " << recv_ghosts_hashed[p][h] << " for proc " << p ;
        hash_to_elements_iter = hash_to_elements.find(recv_ghosts_hashed[p][h]);
        if ( hash_to_elements_iter != hash_not_found )
        {
          //            std::cout << " ---> found " ;
          const Space& entities_space = space(*hash_to_elements_iter->second.comp);
          const Uint elem_idx = hash_to_elements_iter->second.idx;

          if (rank()[entities_space.connectivity()[elem_idx][0]] == PE::Comm::instance().rank()) // if owned
          {
            cf3_assert_desc(to_str(hash_to_elements_iter->second.idx)+" < "+to_str(entities_space.connectivity().size()),
                            hash_to_elements_iter->second.idx < entities_space.connectivity().size());
            cf3_assert(entities_space.connectivity()[ elem_idx ][0] < glb_idx().size());
            Uint first_glb_idx = glb_idx()[ entities_space.connectivity()[ elem_idx ][0] ];
            send_glb_idx_on_rank[p][h] = first_glb_idx;
          }
        }
        //          std::cout << std::endl;
        //          else    // This check only works with 2 processes, as it MUST be found on the other process
        //          {
        //            std::cout << PERank << "hash["<<p<<"]["<<h<<"] = " << recv_ghosts_hashed[p][h] << " not found " << std::endl;
        //            std::cout << "      possible hashes:" << std::endl;
        //            for(hash_to_elem_idx_iter=hash_to_elem_idx.begin(); hash_to_elem_idx_iter!=hash_not_found; ++hash_to_elem_idx_iter)
        //            {
        //              std::cout << " -- " << hash_to_elem_idx_iter->first << std::endl;
        //            }
        //          }
      }
    }
  }

  // (6)
  std::vector< std::vector<Uint> > recv_glb_idx_on_rank(Comm::instance().size());
  if (Comm::instance().is_active())
    Comm::instance().all_to_all(send_glb_idx_on_rank,recv_glb_idx_on_rank);
  else
    recv_glb_idx_on_rank[0] = send_glb_idx_on_rank[0];


  for (Uint g=0; g<ghosts.size(); ++g)
  {
    const Space& entities_space = space(*ghosts[g].comp);
    const Uint elem_idx = ghosts[g].idx;
    cf3_assert(elem_idx<entities_space.connectivity().size());
    cf3_assert(elem_idx<entities_space.support().rank().size());

    // 2 cases:
    // (1) rank = UNKNOWN
    // ---> search for the non-UNKNOWN ghost index
    // (2) rank = KNOWN
    // ---> get the ghost idx from the known rank

    const Uint first_loc_idx = entities_space.connectivity()[elem_idx][0];

    const Uint ghost_rank = rank()[first_loc_idx];
    const Uint first_glb_idx = recv_glb_idx_on_rank[ ghost_rank ][g];

    cf3_assert(ghost_rank < Comm::instance().size());
    if (first_glb_idx == UNKNOWN)
      throw ValueNotFound(FromHere(), "Could  not find ghost element "+entities_space.uri().path()+"["+to_str(elem_idx)+"] with hash "+to_str(ghosts_hashed[g])+" on rank "+to_str(ghost_rank));
    for (Uint s=0; s<entities_space.shape_function().nb_nodes(); ++s)
    {
      cf3_assert(first_loc_idx+s<glb_idx().size());
      glb_idx()[first_loc_idx+s] = first_glb_idx+s;
      rank()[first_loc_idx+s] = ghost_rank;
    }
  }

  create_coordinates();
}
////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

#undef UNKNOWN
