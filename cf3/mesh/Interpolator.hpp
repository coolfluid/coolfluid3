// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Interpolator_hpp
#define cf3_mesh_Interpolator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/AInterpolator.hpp"
#include "mesh/Space.hpp"

namespace cf3 {
namespace mesh {

class APointInterpolator;

////////////////////////////////////////////////////////////////////////////////

/// @brief Interpolator component delegating interpolation to a PointInterpolator
///        strategy
///
/// Allows to interpolate from one field to another field or table.
/// Note that the other field or table does not have to be in the same
/// mesh as the source, depending on concrete implementations
/// The interpolation also works with parallel distributed fields. Interpolation
/// is delegated to the processor that has the necessary source values.
/// @author Willem Deconinck
class Mesh_API Interpolator : public AInterpolator {

public: // functions

  /// Contructor
  /// @param name of the component
  Interpolator ( const std::string& name );

  /// Virtual destructor
  virtual ~Interpolator() {}

  /// Get the class name
  static std::string type_name () { return "Interpolator"; }

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

private: // functions

  void store(const Dictionary& dict, const common::Table<Real>& target_coords);

  void stored_interpolation(const Field& source_field, common::Table<Real>& target);

  void unstored_interpolation(const Field& source_field, const common::Table<Real>& target_coords, common::Table<Real>& target);

protected: // data

  /// The strategy to interpolate one coordinate
  Handle<APointInterpolator>  m_point_interpolator;

private: // data

  Handle<Dictionary const> m_dict;
  Uint m_source_dict_size;
  common::URI m_source_dict_uri;
  Uint m_target_size;

  Handle<common::Table<Real> const> m_table;

  // Values for each processor
  std::vector< int                                   > m_proc;
  std::vector< std::vector< Uint                   > > m_expect_recv;
  std::vector< std::vector< SpaceElem              > > m_stored_element;
  std::vector< std::vector< std::vector<SpaceElem> > > m_stored_stencil;
  std::vector< std::vector< std::vector<Uint>      > > m_stored_source_field_points;
  std::vector< std::vector< std::vector<Real>      > > m_stored_source_field_weights;

  // store variable indices in table rows
  std::vector<Uint> m_source_vars;
  std::vector<Uint> m_target_vars;
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Compile-time configured strategy for interpolation.
///
/// Check InterpolatorTypes.cpp  for  a list of pre-configured types
/// @author Willem Deconinck
template < typename POINTINTERPOLATOR>
class Mesh_API InterpolatorT : public Interpolator {
public:
  /// Contructor
  /// @param name of the component
  InterpolatorT<POINTINTERPOLATOR> ( const std::string& name ) : Interpolator(name)
  {
    remove_component(*m_point_interpolator);
    m_point_interpolator = Handle<APointInterpolator>( create_component<POINTINTERPOLATOR>("point_interpolator") );
  }

  /// Virtual destructor
  virtual ~InterpolatorT<POINTINTERPOLATOR>() {}

  /// Get the class name
  static std::string type_name () { return "InterpolatorT<"+POINTINTERPOLATOR::type_name()+">"; }

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_AAInterpolator_hpp
