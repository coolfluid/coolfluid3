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

  void interpolate(const Field& source_field, Field& target_field);

private: // data

  /// The strategy to interpolate one coordinate
  Handle<PointInterpolator>  m_point_interpolator;

  std::vector< SpaceElem >              m_stored_element;
  std::vector< std::vector<SpaceElem> > m_stored_stencil;
  std::vector< std::vector<Uint>   >    m_stored_source_field_points;
  std::vector< std::vector<Real>   >    m_stored_source_field_weights;

};

////////////////////////////////////////////////////////////////////////////////

/// Interpolator component class
/// This class serves as a component that that will read
/// the mesh format from file
/// @author Willem Deconinck
class Mesh_API OldInterpolator : public common::Component {

public: // functions

  /// Contructor
  /// @param name of the component
  OldInterpolator ( const std::string& name );

  /// Virtual destructor
  virtual ~OldInterpolator();

  /// Get the class name
  static std::string type_name () { return "Interpolator"; }

  // --------- Signals ---------

  void signal_interpolate( common::SignalArgs& node  );

  // --------- Direct access ---------

  void interpolate();

  virtual void construct_internal_storage(Mesh& source) = 0;

  virtual void interpolate_field_from_to(const Field& source, Field& target) = 0;

private: // functions

private: // data

  /// source field
  Handle<Field> m_source;

  /// target field
  Handle<Field> m_target;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Interpolator_hpp
