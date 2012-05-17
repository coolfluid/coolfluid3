// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Interpolator_hpp
#define cf3_mesh_Interpolator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

#include "mesh/LibMesh.hpp"
#include "mesh/Space.hpp"


namespace cf3 {
namespace mesh {

  class Mesh;
  class Field;
  class PointInterpolator;

////////////////////////////////////////////////////////////////////////////////

/// Interpolator component class
/// This class serves as a component that that will read
/// the mesh format from file
/// @author Willem Deconinck
class Mesh_API Interpolator : public common::Component {

public: // functions

  /// Contructor
  /// @param name of the component
  Interpolator ( const std::string& name );

  /// Virtual destructor
  virtual ~Interpolator() {}

  /// Get the class name
  static std::string type_name () { return "Interpolator"; }

  void store(const Dictionary& dict, const common::Table<Real>& target_coords);

  void interpolate(const Field& source_field, Field& target_field);

  void stored_interpolation(const Field& source_field, common::Table<Real>& target);

  void unstored_interpolation(const Field& source_field, const common::Table<Real>& target_coords, common::Table<Real>& target);

private: // data

  /// The strategy to interpolate one coordinate
  Handle<PointInterpolator>  m_point_interpolator;

  Handle<Dictionary const> m_dict;
  Handle<common::Table<Real> const> m_table;

  // Values for each processor
  std::vector< int                                   > m_proc;
  std::vector< std::vector< Uint                   > > m_expect_recv;
  std::vector< std::vector< SpaceElem              > > m_stored_element;
  std::vector< std::vector< std::vector<SpaceElem> > > m_stored_stencil;
  std::vector< std::vector< std::vector<Uint>      > > m_stored_source_field_points;
  std::vector< std::vector< std::vector<Real>      > > m_stored_source_field_weights;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Interpolator_hpp
