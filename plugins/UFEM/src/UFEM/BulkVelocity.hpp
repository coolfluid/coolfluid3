// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_BulkVelocity_hpp
#define cf3_UFEM_BulkVelocity_hpp

#include "solver/History.hpp"

#include "solver/actions/Proto/DirichletBC.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"


#include "LibUFEM.hpp"

namespace cf3 {
  namespace math { namespace LSS { class System; } }
  namespace mesh { class Region; }
namespace UFEM {

/// Calculates the surface integral of a variable.
/// The result of the integral is returned through the result option
/// A History component can be set to log the value.
class UFEM_API BulkVelocity : public solver::actions::Proto::ProtoAction
{
public:

  /// Contructor
  /// @param name of the component
  BulkVelocity ( const std::string& name );
  
  virtual ~BulkVelocity();

  /// Get the class name
  static std::string type_name () { return "BulkVelocity"; }
  
  /// Set up the field to use. This builds either a vector or a scalar expression, depending on the variable passed
  void set_field(const std::string& variable_name, const std::string& tag);
  
  void signal_set_field(common::SignalArgs& args);
  void signature_set_field(common::SignalArgs& args);
  
  virtual void execute();
  
  /// Return the result
  Real result()
  {
    return m_integral_value;
  }
private:
  /// Trigger on a change of the result
  void trigger_result();

  /// Name of the variable to use
  std::string m_variable_name;
  
  /// Storage for the value of the surface integral
  Real m_integral_value;
  Real m_area;
  Real m_result;
  
  Handle<solver::History> m_history;
  bool m_changing_result;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_BulkVelocity_hpp
