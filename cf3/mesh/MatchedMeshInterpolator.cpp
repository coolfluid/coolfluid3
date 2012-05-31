// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/FindComponents.hpp"
#include "common/Builder.hpp"

#include "math/MatrixTypesConversion.hpp"
#include "math/Hilbert.hpp"

#include "mesh/BoundingBox.hpp"
#include "mesh/MatchedMeshInterpolator.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

namespace cf3 {
namespace mesh {

using namespace cf3::common;
using namespace cf3::math;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder<MatchedMeshInterpolator,AInterpolator,LibMesh> MatchedMeshInterpolator_builder;

////////////////////////////////////////////////////////////////////////////////

MatchedMeshInterpolator::MatchedMeshInterpolator(const std::string &name) : AInterpolator(name)
{
}

////////////////////////////////////////////////////////////////////////////////

void MatchedMeshInterpolator::interpolate_vars(const Field& source_field, const common::Table<Real>& target_coords, common::Table<Real>& target, const std::vector<Uint>& source_vars, const std::vector<Uint>& target_vars)
{
  if ( is_null( target.handle<Field>() ) )
    throw common::SetupError(FromHere(),"Target must be a field");

  Field& target_field = *target.handle<Field>();
  Dictionary& target_dict = target_field.dict();
  Mesh& target_mesh = find_parent_component<Mesh>(target_dict);

  const Dictionary& source_dict = source_field.dict();
  const Mesh& source_mesh = find_parent_component<Mesh>(source_dict);

  Hilbert compute_hilbert_idx (*source_mesh.global_bounding_box(),20);

  // Create element glb_idx map
  std::map< boost::uint64_t , SpaceElem > find_source_element;
  boost_foreach( const Handle<Space>& space, source_dict.spaces())
  {
    RealMatrix nodes;
    space->support().geometry_space().allocate_coordinates(nodes);
    RealVector centroid(space->support().element_type().dimension());
    Uint e=0;
    space->support().geometry_space().put_coordinates(nodes,e);
    space->support().element_type().compute_centroid(nodes,centroid);
    find_source_element [ compute_hilbert_idx(centroid) ] = SpaceElem(*space,e);
  }

  /// @todo Add parallel distributed support

  const Uint nb_vars=source_vars.size();

  /// Loop over Regions
  boost_foreach(const Handle<Space>& t_space, target_field.dict().spaces())
  {
    const ShapeFunction& t_sf = t_space->shape_function();

    RealMatrix nodes;
    t_space->support().geometry_space().allocate_coordinates(nodes);
    RealVector centroid(t_space->support().element_type().dimension());

    /// Element loop
    const Uint nb_elems = t_space->size();

    // Since the mesh matches, the s_space must also remain the same
    if (nb_elems>0)
    {
      t_space->support().geometry_space().put_coordinates(nodes,0);
      t_space->support().element_type().compute_centroid(nodes,centroid);

      const SpaceElem      s_elem  = find_source_element[ compute_hilbert_idx(centroid) ];
      const Space*         s_space = s_elem.comp;
      const ShapeFunction& s_sf    = s_elem.shape_function();

      /// Compute Interpolation matrix, equal for every element
      RealMatrix interpolate(t_sf.nb_nodes(),s_sf.nb_nodes());
      for (Uint t_pt = 0; t_pt<t_sf.nb_nodes(); ++t_pt)
        interpolate.row(t_pt) = s_sf.value(t_sf.local_coordinates().row(t_pt));

      for (Uint e=0; e<nb_elems; ++e)
      {
        Connectivity::ConstRow s_field_indexes = s_space->connectivity()[e];
        Connectivity::ConstRow t_field_indexes = t_space->connectivity()[e];

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
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
