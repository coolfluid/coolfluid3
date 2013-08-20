// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_VolumeIntegral_hpp
#define cf3_mesh_actions_VolumeIntegral_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Action.hpp"
#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  class Region;
  class Field;
  class Entities;
  class Quadrature;
  
namespace actions {

//////////////////////////////////////////////////////////////////////////////

/// @brief VolumeIntegral Fields with matching support
///
/// @post After this, the mesh is ready to be parallellized
/// @author Willem Deconinck
class mesh_actions_API VolumeIntegral : public common::Action {

public: // functions

  /// constructor
  VolumeIntegral( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "VolumeIntegral"; }

  virtual void execute();

  /// VolumeIntegral of a field
  /// @param [in]  field       integrate this field
  /// @param [in]  regions     integrate Volume regions contained
  /// @result Value of integration
  Real integrate(const Field& field, const std::vector< Handle<Region> >& regions);

  /// VolumeIntegral of a field
  /// @param [in]  field        integrate this field
  /// @param [in]  entities     integrate Volume entities
  /// @result Value of integration
  Real integrate(const Field& field, const std::vector< Handle<Entities> >& entities);
    
  void signal_integrate ( common::SignalArgs& node);
  void signature_integrate ( common::SignalArgs& node);

private:

  Uint m_order;
  Handle<Field> m_field;
  std::vector< Handle<Region> > m_regions;
  Handle<Quadrature> m_quadrature;

}; // end VolumeIntegral


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_VolumeIntegral_hpp
