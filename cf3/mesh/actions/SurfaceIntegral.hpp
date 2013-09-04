// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_SurfaceIntegral_hpp
#define cf3_mesh_actions_SurfaceIntegral_hpp

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

/// @brief SurfaceIntegral Fields with matching support
///
/// @post After this, the mesh is ready to be parallellized
/// @author Willem Deconinck
class mesh_actions_API SurfaceIntegral : public common::Action {

public: // functions

  /// constructor
  SurfaceIntegral( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "SurfaceIntegral"; }

  virtual void execute();

  /// SurfaceIntegral of a field
  /// @param [in]  field       integrate this field
  /// @param [in]  regions     integrate surface regions contained
  /// @result Value of integration
  Real integrate(const Field& field, const std::vector< Handle<Region> >& regions);

  /// SurfaceIntegral of a field
  /// @param [in]  field        integrate this field
  /// @param [in]  entities     integrate surface entities
  /// @result Value of integration
  Real integrate(const Field& field, const std::vector< Handle<Entities> >& entities);
    
  void signal_integrate ( common::SignalArgs& node);
  void signature_integrate ( common::SignalArgs& node);

private:

  Uint m_order;
  Handle<Field> m_field;
  std::vector< Handle<Region> > m_regions;
  Handle<Quadrature> m_quadrature;

}; // end SurfaceIntegral


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_SurfaceIntegral_hpp
