// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_LSSActionUnsteady_hpp
#define cf3_UFEM_LSSActionUnsteady_hpp

#include "solver/Time.hpp"

#include "LibUFEM.hpp"
#include "LSSAction.hpp"

namespace cf3 {
namespace solver { class Time; }
namespace UFEM {

/// LSSActionUnsteady for UFEM problems, allowing dynamic configuration and providing access to
/// * Linear system solver
/// * Physical model
/// * Mesh used
/// * Region to loop over
class UFEM_API LSSActionUnsteady : public LSSAction
{
public: // functions

  /// Contructor
  /// @param name of the component
  LSSActionUnsteady ( const std::string& name );

  virtual ~LSSActionUnsteady();

  /// Get the class name
  static std::string type_name () { return "LSSActionUnsteady"; }

  /// Reference to the timestep
  Real& dt();

  /// Reference to the inverse timestep, linked to the model time step
  Real& invdt();

  const solver::Time& time() const;

private:

  void trigger_time();
  void trigger_timestep();

  Handle<solver::Time> m_time;
  Real m_dt, m_invdt;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_LSSActionUnsteady_hpp
