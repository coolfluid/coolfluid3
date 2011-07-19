// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CTime_hpp
#define CF_Solver_CTime_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Solver/LibSolver.hpp"

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// @brief Storage for time, and time steps for unsteady simulation
///
/// This class keeps track of time for use in unsteady
/// simulations
/// @author Willem Deconinck
class Solver_API CTime : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CTime> Ptr;
  typedef boost::shared_ptr<CTime const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CTime ( const std::string& name );

  /// Virtual destructor
  virtual ~CTime();

  /// Get the class name
  static std::string type_name () { return "CTime"; }

  /// @return modifiable time
  Real& current_time() { return m_current_time; }

  /// @return time
  const Real& current_time() const { return m_current_time; }

  /// @return modifiable time step
  Real& dt() { return m_dt; }

  /// @return time step
  const Real& dt() const { return m_dt; }

  /// @return Inverse timestep, updated automatically when the time step is changed
  const Real& invdt() const { return m_invdt; }

  /// @return Inverse timestep, updated automatically when the time step is changed
  Real& invdt() { return m_invdt; }

  /// @return modifiable iteration
  Uint& iter() { return m_iter; }

  /// @return iteration
  const Uint& iter() const { return m_iter; }

  /// @return end_time
  Real end_time() const { return m_end_time; }

private: // data

  Real m_current_time;
  Real m_dt;
  Real m_invdt;
  Uint m_iter;
  Real m_end_time;

  void trigger_timestep();
};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CTime_hpp
