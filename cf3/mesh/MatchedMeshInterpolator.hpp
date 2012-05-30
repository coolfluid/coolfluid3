// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_MatchedMeshInterpolator_hpp
#define cf3_mesh_MatchedMeshInterpolator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/AInterpolator.hpp"

namespace cf3 {
namespace mesh {

class APointInterpolator;

////////////////////////////////////////////////////////////////////////////////

/// @brief Interpolator component that interpolates fields between spaces in the same mesh
///
/// @author Willem Deconinck
class Mesh_API MatchedMeshInterpolator : public AInterpolator {

public: // functions

  /// Contructor
  /// @param name of the component
  MatchedMeshInterpolator ( const std::string& name );

  /// Virtual destructor
  virtual ~MatchedMeshInterpolator() {}

  /// Get the class name
  static std::string type_name () { return "MatchedMeshInterpolator"; }

  /// @brief Interpolate specified variables from a source field to a target table
  ///
  /// The source_field and target_field don't have to have identical variable numbering, or even the amount
  /// of variables. A mapping from source_vars to target_vars must be given as arguments.
  /// The source_vars and target_vars arguments contain absolute indices, meaning a vector variable in 3D must be given by 3 indices.
  /// @param [in]  source_field   Field to interpolate from
  /// @param [in]  target_coords  Table with coordinates to interpolate to
  /// @param [out] target         Table to interpolate to
  /// @param [in]  source_vars    Variable indices from source_field to interpolate from
  /// @param [in]  target_vars    Variables in target_field to interpolate to
  virtual void interpolate_vars(const Field& source_field, const common::Table<Real>& target_coords, common::Table<Real>& target, const std::vector<Uint>& source_vars, const std::vector<Uint>& target_vars);
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_MatchedMeshInterpolator_hpp
