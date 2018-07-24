// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_SurfaceIntegral_hpp
#define cf3_UFEM_SurfaceIntegral_hpp

#include "solver/History.hpp"

#include "solver/Action.hpp"

#include "LibUFEM.hpp"

namespace cf3 {
namespace UFEM {

/// Calculates the surface integral of a variable.
/// The result of the integral is returned through the result option
/// A History component can be set to log the value.
class UFEM_API SurfaceIntegral : public solver::Action
{
public:

  /// Contructor
  /// @param name of the component
  SurfaceIntegral ( const std::string& name );

  virtual ~SurfaceIntegral();

  /// Get the class name
  static std::string type_name () { return "SurfaceIntegral"; }

  virtual void execute();

  /// Return the result
  const RealVector& result()
  {
    return m_integral_value;
  }
private:
  /// Trigger on a change of the result
  void trigger_result();

  /// Name of the variable to use
  std::string m_variable_name;

  /// Storage for the value of the surface integral
  RealVector m_integral_value;

  Handle<solver::History> m_history;
  bool m_changing_result;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_SurfaceIntegral_hpp
