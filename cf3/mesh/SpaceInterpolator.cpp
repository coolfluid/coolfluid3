// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "math/MatrixTypesConversion.hpp"

#include "common/FindComponents.hpp"
#include "common/Builder.hpp"

#include "mesh/SpaceInterpolator.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

namespace cf3 {
namespace mesh {

using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder<SpaceInterpolator,AInterpolator,LibMesh> SpaceInterpolator_builder;

////////////////////////////////////////////////////////////////////////////////

SpaceInterpolator::SpaceInterpolator(const std::string &name) : AInterpolator(name)
{
}

////////////////////////////////////////////////////////////////////////////////

void SpaceInterpolator::interpolate_vars(const Field& source_field, const common::Table<Real>& target_coords, common::Table<Real>& target, const std::vector<Uint>& source_vars, const std::vector<Uint>& target_vars)
{
  if ( is_null( target.handle<Field>() ) )
    throw common::SetupError(FromHere(),"Target must be a field");

  Field& target_field = *target.handle<Field>();

  Handle<Mesh> source_mesh = find_parent_component_ptr<Mesh>(source_field);
  Handle<Mesh> target_mesh = find_parent_component_ptr<Mesh>(target_field);

  if ( source_mesh != target_mesh )
  {
    throw common::SetupError(FromHere(),"This optimized algorithm needs the source mesh to be the same as "
                             "the target mesh. \nUse another interpolator for mesh to mesh interpolation.");
  }

  if (source_vars.size() != target_vars.size())
  {
    throw common::SetupError(FromHere(),"Sizes of source_vars and target_vars don't match");
  }

  const Uint nb_vars=source_vars.size();

  /// Loop over Regions
  boost_foreach(const Handle<Entities>& elements_handle, target_field.dict().entities_range())
  {
    Entities& elements = *elements_handle;
    if (source_field.dict().defined_for_entities(elements_handle) == false)
      continue;
    //      throw BadValue(FromHere(),"Source field "+source.uri().string()+" is not defined in elements "+elements.uri().string());

    const Space& s_space = source_field.space(elements);
    const Space& t_space = target_field.space(elements);
    const ShapeFunction& s_sf = s_space.shape_function();
    const ShapeFunction& t_sf = t_space.shape_function();

    /// Compute Interpolation matrix, equal for every element
    RealMatrix interpolate(t_sf.nb_nodes(),s_sf.nb_nodes());
    for (Uint t_pt = 0; t_pt<t_sf.nb_nodes(); ++t_pt)
      interpolate.row(t_pt) = s_sf.value(t_sf.local_coordinates().row(t_pt));

    /// Element loop
    for (Uint e=0; e<elements.size(); ++e)
    {
      Connectivity::ConstRow s_field_indexes = s_space.connectivity()[e];
      Connectivity::ConstRow t_field_indexes = t_space.connectivity()[e];

      /// Interpolate: target[element] = interpolate * source[element]
      /// Split in loops since we cannot work with Matrix-products
      for (Uint t_pt=0; t_pt<t_sf.nb_nodes(); ++t_pt)
      {
        for (Uint var=0; var<nb_vars; ++var)
          target_field[t_field_indexes[t_pt]][target_vars[var]] = 0.;

        for (Uint s_pt=0; s_pt<s_sf.nb_nodes(); ++s_pt)
        {
          for (Uint var=0; var<nb_vars; ++var)
            target_field[t_field_indexes[t_pt]][target_vars[var]] += interpolate(t_pt,s_pt) * source_field[s_field_indexes[s_pt]][source_vars[var]];
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
