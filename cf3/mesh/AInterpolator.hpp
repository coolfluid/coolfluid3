// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_AInterpolator_hpp
#define cf3_mesh_AInterpolator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"
#include "common/Table_fwd.hpp"
#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace mesh {

  class Mesh;
  class Field;
  
////////////////////////////////////////////////////////////////////////////////

/// @brief A Interpolator base component
///
/// Allows to interpolate from one field to another field or table.
/// Note that the other field or table does not have to be in the same
/// mesh as the source, depending on concrete implementations
/// The interpolation also works with parallel distributed fields. Interpolation
/// is delegated to the processor that has the necessary source values.
/// @author Willem Deconinck
class Mesh_API AInterpolator : public common::Component {

public: // functions

  /// Contructor
  /// @param name of the component
  AInterpolator ( const std::string& name );

  /// Virtual destructor
  virtual ~AInterpolator() {}

  /// Get the class name
  static std::string type_name () { return "AInterpolator"; }

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
  virtual void interpolate_vars(const Field& source_field, const common::Table<Real>& target_coords, common::Table<Real>& target, const std::vector<Uint>& source_vars, const std::vector<Uint>& target_vars) = 0;

  /// @brief Interpolate specified variables from a source field to a target field
  ///
  /// The source_field and target_field don't have to have identical variable numbering, or even the amount
  /// of variables. A mapping from source_vars to target_vars must be given as arguments.
  /// The source_vars and target_vars arguments contain absolute indices, meaning a vector variable in 3D must be given by 3 indices.
  /// @param [in]  source_field   Field to interpolate from
  /// @param [out] target_field   Field to interpolate to
  /// @param [in]  source_vars    Variable indices from source_field to interpolate from
  /// @param [in]  target_vars    Variables in target_field to interpolate to
  void interpolate_vars(const Field& source_field, Field& target_field, const std::vector<Uint>& source_vars, const std::vector<Uint>& target_vars);

  /// @brief Interpolate from a field to another field
  ///
  /// Wrapper function to call interpolate_vars(). The source_field and target_field must have identical
  /// variable numbering and row size.
  /// @param [in]  source_field   Field to interpolate from
  /// @param [out] target_field   Field to interpolate to
  void interpolate(const Field& source_field, Field& target_field);

  /// @brief Interpolate from a field to a table
  ///
  /// Wrapper function to call interpolate_vars(). The source_field and target must have identical
  /// variable numbering and row size.
  /// @param [in]  source_field   Field to interpolate from
  /// @param [in]  target_coords  Table with coordinates to interpolate to
  /// @param [out] target         Table where interpolated values are stored. (same amount of rows as target_coords)
  void interpolate(const Field& source_field, const common::Table<Real>& target_coords, common::Table<Real>& target);

  /// @name SIGNALS
  //@{
  void signal_interpolate    ( common::SignalArgs& node);
  void signature_interpolate ( common::SignalArgs& node);
  //@}

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_AInterpolator_hpp
