// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"

#include "common/PE/Comm.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshAdaptor.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Field.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/PointInterpolator.hpp"

#include "MeshInterpolator.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < MeshInterpolator, common::Action, mesh::actions::LibActions> MeshInterpolator_Builder;

////////////////////////////////////////////////////////////////////////////////

MeshInterpolator::MeshInterpolator(const std::string& name) : common::Action(name)
{
  options().add("source_mesh", Handle<Mesh>())
    .pretty_name("Source Mesh")
    .description("Mesh to interpolate from. Warning: this is modified in parallel to make all elements known on each rank.")
    .mark_basic();
    
  options().add("target_mesh", Handle<Mesh>())
    .pretty_name("Target Mesh")
    .description("Mesh to interpolate to")
    .mark_basic();
    
  create_static_component<PointInterpolator>("PointInterpolator");
}

void MeshInterpolator::execute()
{
  Handle<Mesh> source_mesh = options().value< Handle<Mesh> >("source_mesh");
  Handle<Mesh> target_mesh = options().value< Handle<Mesh> >("target_mesh");
  
  if(is_null(source_mesh))
    throw common::SetupError(FromHere(), "MeshInterpolator source_mesh is not set");
  
  if(is_null(target_mesh))
    throw common::SetupError(FromHere(), "MeshInterpolator target_mesh is not set");
  
  common::PE::Comm& comm = common::PE::Comm::instance();
  const Uint nb_procs = comm.size();
  const Uint my_rank = comm.rank();
  
  Handle<PointInterpolator> point_interpolator(get_child("PointInterpolator"));
  cf3_always_assert(is_not_null(point_interpolator));
  
  BOOST_FOREACH(const Handle<Dictionary>& source_dict, source_mesh->dictionaries())
  {
    if(source_dict->discontinuous())
      continue;
    
    point_interpolator->options().set("dict", source_dict);
    
    Handle<Dictionary> target_dict(target_mesh->get_child(source_dict->name()));
    if(is_null(target_dict))
    {
      target_dict = target_mesh->create_continuous_space(source_dict->name(), source_dict->properties().value<std::string>("space_lib_name")).handle<Dictionary>();
      BOOST_FOREACH(const std::string& tag, source_dict->get_tags())
      {
        target_dict->add_tag(tag);
      }
    }
    
    const Field& source_coords = source_dict->coordinates();
    const Field& target_coords = target_dict->coordinates();
    const Uint nb_target_points = target_coords.size();
    const Uint dim = target_coords.row_size();
    std::vector<SpaceElem> space_elems(nb_target_points);
    std::vector<Uint> points_begin_idxs(nb_target_points, 0);
    std::vector<Uint> points_end_idxs(nb_target_points, 0);
    std::vector<Uint> all_points; all_points.reserve(nb_target_points*8);
    std::vector<Real> all_weights; all_weights.reserve(nb_target_points*8);
    std::vector<Real> my_missing_points; my_missing_points.reserve(nb_target_points/10);
    
    
    for(Uint i = 0; i != nb_target_points; ++i)
    {
      std::vector<Uint> points;
      std::vector<Real> weights;
      std::vector<SpaceElem> dummy_stencil;
      Field::ConstRow coordrow = target_coords[i];
      Eigen::Map<RealVector const> coord(&coordrow[0], dim);
      bool found = point_interpolator->compute_storage(coord, space_elems[i], dummy_stencil, points, weights);
      if(!found)
      {
        my_missing_points.insert(my_missing_points.end(), coordrow.begin(), coordrow.end());
      }
    }

    if(comm.size() > 1)
    {
      MeshAdaptor adaptor(*source_mesh);
      adaptor.prepare();
      std::vector< std::vector< std::vector<Uint> > > elements_to_send(nb_procs, std::vector< std::vector<Uint> > (source_mesh->elements().size()));

      std::vector< std::vector<Real> > recv_missing_points;
      comm.all_gather(my_missing_points, recv_missing_points);
      const Uint nb_ranks = comm.size();
      cf3_assert(recv_missing_points.size() == nb_ranks);

      for(Uint rank = 0; rank != nb_ranks; ++rank)
      {
        if(rank == comm.rank())
          continue;
        const std::vector<Real>& other_missing_points = recv_missing_points[rank];
        cf3_assert(other_missing_points.size() % dim == 0);
        const Uint missing_end = other_missing_points.size() / dim;
        for(Uint missing_idx = 0; missing_idx != missing_end; ++missing_idx)
        {
          std::vector<Uint> points;
          std::vector<Real> weights;
          std::vector<SpaceElem> dummy_stencil;
          SpaceElem space_elem;
          Eigen::Map<RealVector const> coord(&other_missing_points[missing_idx*dim], dim);
          bool found = point_interpolator->compute_storage(coord, space_elem, dummy_stencil, points, weights);
          if(found && !space_elem.is_ghost())
          {
            elements_to_send[rank][space_elem.comp->support().entities_idx()].push_back(space_elem.idx);
          }
        }
      }

      std::vector< std::vector< std::vector<Uint> > > nodes_to_send;
      adaptor.find_nodes_to_export(elements_to_send,nodes_to_send);

      std::vector< std::vector< std::vector<boost::uint64_t> > > imported_elements;
      adaptor.send_elements(elements_to_send, imported_elements);

      adaptor.flush_elements();

      std::vector< std::vector< std::vector<boost::uint64_t> > > imported_nodes;
      adaptor.send_nodes(nodes_to_send,imported_nodes);

      adaptor.flush_nodes();

      adaptor.finish();

      CFinfo << "  + growing overlap layer ..." << CFendl;
      common::build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GrowOverlap","grow_overlap")->transform(*source_mesh);
      CFinfo << "  + growing overlap layer ... done" << CFendl;

      point_interpolator->remove_tag(common::Tags::static_component());
      remove_component(*point_interpolator);
      source_mesh->remove_component("octtree");
      point_interpolator = create_static_component<PointInterpolator>("PointInterpolator");
      point_interpolator->options().set("function", std::string("cf3.mesh.ShapeFunctionInterpolation"));
      point_interpolator->options().set("dict", source_dict);
    }

    // Sometimes a point still is not found, so we slightly perturb the coordinates
    std::vector<RealVector> perturbations(dim*2, RealVector(dim));
    for(Uint i = 0; i != dim; ++i)
    {
      perturbations[2*i  ].setZero();
      perturbations[2*i+1].setZero();
      perturbations[2*i  ][i] = 1e-8;
      perturbations[2*i+1][i] = -1e-8;
    }

    for(Uint i = 0; i != nb_target_points; ++i)
    {
      std::vector<Uint> points;
      std::vector<Real> weights;
      std::vector<SpaceElem> dummy_stencil;
      Field::ConstRow coordrow = target_coords[i];
      Eigen::Map<RealVector const> coord(&coordrow[0], dim);
      bool found = point_interpolator->compute_storage(coord, space_elems[i], dummy_stencil, points, weights);
      if(!found)
      {
        BOOST_FOREACH(const RealVector& perturbation, perturbations)
        {
          found = point_interpolator->compute_storage(coord+perturbation, space_elems[i], dummy_stencil, points, weights);
          if(found)
            break;
        }
      }
      if(!found && !target_dict->is_ghost(i))
      {
        CFerror << " Point " << coord.transpose() << " was not found in source mesh" << CFendl;
      }
      else
      {
        points_begin_idxs[i] = all_points.size();
        all_points.insert(all_points.end(), points.begin(), points.end());
        all_weights.insert(all_weights.end(), weights.begin(), weights.end());
        points_end_idxs[i] = all_points.size();
      }
    }

    BOOST_FOREACH(const Handle<Field>& source_field, source_dict->fields())
    {
      if(source_field->has_tag(mesh::Tags::coordinates()))
        continue;

      Handle<Field> target_field(target_dict->get_child(source_field->name()));
      if(is_null(target_field))
      {
        target_field = target_dict->create_field(source_field->name(), source_field->descriptor().description()).handle<Field>();
        BOOST_FOREACH(const std::string& tag, source_field->get_tags())
        {
          target_field->add_tag(tag);
        }
      }

      const Field::ArrayT& source_array = source_field->array();
      Field::ArrayT& target_array = target_field->array();
      const Uint row_size = source_field->row_size();
      cf3_always_assert(row_size == target_field->row_size());
      cf3_assert(nb_target_points == target_array.size());

      for(Uint i = 0; i != nb_target_points; ++i)
      {
        if(target_dict->is_ghost(i))
          continue;
        Eigen::Map<RealVector> target_row(&target_array[i][0], row_size);
        target_row.setZero();
        RealVector avg_row(target_row);
        const Uint interp_begin = points_begin_idxs[i];
        const Uint interp_end = points_end_idxs[i];
        cf3_assert(interp_begin < all_points.size());
        cf3_assert(interp_end <= all_points.size());
        Real weightsum = 0;
        for(Uint j = interp_begin; j != interp_end; ++j)
        {
          if(all_points[j] >= source_array.size())
          {
            throw common::SetupError(FromHere(), "Point " + common::to_str(all_points[j]) + " is outside the source point range");
          }
          Eigen::Map<RealVector const> source_row(&source_array[all_points[j]][0], row_size);
          target_row += all_weights[j] * source_row;
          avg_row += source_row;
          weightsum += all_weights[j];
        }
        if(weightsum > 1. + 1e-10)
        {
          CFerror << "Bad weights found, using unweighted average on point " << i << CFendl;
          target_row = avg_row;
        }
      }

      target_field->parallelize();
      target_field->synchronize();
    }
  }
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
