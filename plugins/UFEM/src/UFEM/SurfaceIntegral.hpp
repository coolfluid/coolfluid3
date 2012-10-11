// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_SurfaceIntegral_hpp
#define cf3_UFEM_SurfaceIntegral_hpp

#include "solver/History.hpp"

#include "solver/actions/Proto/DirichletBC.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"


#include "LibUFEM.hpp"

namespace cf3 {
  namespace math { namespace LSS { class System; } }
  namespace mesh { class Region; }
namespace UFEM {

/// Calculates the surface integral of a variable. A tag and variable name for the field must be set in the options,
/// as well as a History component to log the value.
class UFEM_API SurfaceIntegral : public solver::actions::Proto::ProtoAction
{
public:

  /// Contructor
  /// @param name of the component
  SurfaceIntegral ( const std::string& name );
  
  virtual ~SurfaceIntegral();

  /// Get the class name
  static std::string type_name () { return "SurfaceIntegral"; }
  
  virtual void execute();
private:
  /// Trigger that sets the expression when one of the relevant options changed
  void trigger_set_expression();

  /// Name of the variable to use
  std::string m_variable_name;
  
  /// Storage for the value of the surface integral
  RealVector m_integral_value;
  
  Handle<solver::History> m_history;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_SurfaceIntegral_hpp
