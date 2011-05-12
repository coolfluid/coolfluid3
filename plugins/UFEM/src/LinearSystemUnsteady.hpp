// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_LinearSystemUnsteady_hpp
#define CF_UFEM_LinearSystemUnsteady_hpp

#include "Solver/Actions/Proto/Terminals.hpp"

#include "LibUFEM.hpp"
#include "LinearSystem.hpp"

namespace CF {
namespace UFEM {

/// Wizard to set up steady linear heat conduction
class UFEM_API LinearSystemUnsteady : public LinearSystem
{
public: // typedefs

  typedef boost::shared_ptr<LinearSystemUnsteady> Ptr;
  typedef boost::shared_ptr<LinearSystemUnsteady const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  LinearSystemUnsteady ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "LinearSystemUnsteady"; }
  
  /// get the timestep inverse. This returns a "StoredReference", so calling this inside a proto expression ensures
  /// the timestep value set through the option is always used.
  Solver::Actions::Proto::StoredReference<Real const> invdt() const
  {
    return Solver::Actions::Proto::store(m_invdt);
  }
  
protected:
  virtual void on_solve();  
  
private:
  
  void trigger_timestep();
  
  /// Inverse timestep
  Real m_invdt;
  
  /// Starting time
  Real m_start_time;
  
  /// Stop time
  Real m_stop_time;
  
  /// Current time
  Real m_current_time;
};

} // UFEM
} // CF


#endif // CF_UFEM_LinearSystemUnsteady_hpp
