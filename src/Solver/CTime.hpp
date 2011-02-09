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
  Real& time() { return m_time; }

  /// @return time
  const Real& time() const { return m_time; }

  /// @return modifiable time step
  Real& dt() { return m_dt; }

  /// @return time step
  const Real& dt() const { return m_dt; }

private: // data

  Real m_time;
  
  Real m_dt;
};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CTime_hpp
