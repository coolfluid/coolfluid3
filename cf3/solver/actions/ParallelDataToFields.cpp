// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/List.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Connectivity.hpp"

#include "solver/actions/ParallelDataToFields.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ParallelDataToFields, common::Action, LibActions > ParallelDataToFields_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ParallelDataToFields::ParallelDataToFields ( const std::string& name ) :
  Action(name)
{
}

/////////////////////////////////////////////////////////////////////////////////////

void ParallelDataToFields::execute()
{
  Handle<mesh::Mesh> mesh_handle = options().value< Handle<mesh::Mesh> >("mesh");
  if(is_null(mesh_handle))
  {
    throw common::SetupError(FromHere(), "Mesh not set for " + uri().path());
  }

  mesh::Mesh& mesh = *mesh_handle;

  // Store element ranks
  Handle<mesh::Dictionary> elems_P0_handle(mesh.get_child("elems_P0"));
  mesh::Dictionary& elems_P0 = is_null(elems_P0_handle) ? mesh.create_discontinuous_space("elems_P0","cf3.mesh.LagrangeP0") : *elems_P0_handle;
  mesh::Field& elem_rank = elems_P0.get_child("element_rank") ? *(elems_P0.get_child("element_rank")->handle<mesh::Field>()) : elems_P0.create_field("element_rank");
  mesh::Field& elem_ghost = elems_P0.get_child("element_ghosts") ? *(elems_P0.get_child("element_ghosts")->handle<mesh::Field>()) : elems_P0.create_field("element_ghosts");

  boost_foreach(const Handle<mesh::Entities>& elements_handle, elems_P0.entities_range())
  {
    mesh::Entities& elements = *elements_handle;
    const mesh::Space& space = elems_P0.space(elements);
    for (Uint elem=0; elem<elements.size(); ++elem)
    {
      Uint field_idx = space.connectivity()[elem][0];
      elem_rank[field_idx][0] = elements.rank()[elem];
      elem_ghost[field_idx][0] = elements.is_ghost(elem) ? 1. : 0.;
    }
  }

  mesh::Dictionary& geom = mesh.geometry_fields();
  mesh::Field& node_rank = geom.get_child("node_rank") ? *(geom.get_child("node_rank")->handle<mesh::Field>()) : geom.create_field("node_rank");
  mesh::Field& node_ghost = geom.get_child("node_ghosts") ? *(geom.get_child("node_ghosts")->handle<mesh::Field>()) : geom.create_field("node_ghosts");
  mesh::Field& node_gids = geom.get_child("node_gids") ? *(geom.get_child("node_gids")->handle<mesh::Field>()) : geom.create_field("node_gids");

  const Uint nb_points = geom.size();
  for(Uint i = 0; i != nb_points; ++i)
  {
    node_rank[i][0] = geom.rank()[i];
    node_ghost[i][0] = static_cast<Real>(geom.is_ghost(i));
    node_gids[i][0] = static_cast<Real>(geom.glb_idx()[i]);
  }

  if(common::PE::Comm::instance().is_active() && common::PE::Comm::instance().size() > 1)
  {
    Uint max_nb_nodes, min_nb_nodes, total_nb_nodes;
    common::PE::Comm::instance().all_reduce(common::PE::min(), &nb_points, 1, &min_nb_nodes);
    common::PE::Comm::instance().all_reduce(common::PE::plus(), &nb_points, 1, &total_nb_nodes);
    common::PE::Comm::instance().all_reduce(common::PE::max(), &nb_points, 1, &max_nb_nodes);

    const Uint mean_nb_nodes = total_nb_nodes / common::PE::Comm::instance().size();
    CFinfo << "Parallel nodes distribution min: " << min_nb_nodes << " (" << static_cast<Real>(min_nb_nodes)/static_cast<Real>(total_nb_nodes)*100. << "%)"
           << ", max: " << max_nb_nodes << " (" << static_cast<Real>(max_nb_nodes)/static_cast<Real>(total_nb_nodes)*100. << "%)"
           << ", mean: " << mean_nb_nodes << " (" << static_cast<Real>(mean_nb_nodes)/static_cast<Real>(total_nb_nodes)*100. << "%)" << CFendl;
  }
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

