// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_Solver_hpp
#define CF_RDM_Solver_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLink.hpp"

#include "Solver/CSolver.hpp"
#include "Solver/Action.hpp"

#include "RDM/Core/LibCore.hpp"

namespace CF {

namespace Mesh    { class CField;    class CMesh; }
namespace Physics { class PhysModel; class Variables; }
namespace Solver  { namespace Actions { class CSynchronizeFields; } }

namespace RDM {
namespace Core {

class BoundaryConditions;
class InitialConditions;
class DomainDiscretization;
class IterativeSolver;
class TimeStepping;

////////////////////////////////////////////////////////////////////////////////

/// RDM Solver
///
/// @author Tiago Quintino
/// @author Martin Vymazal
/// @author Mario Ricchiuto
/// @author Willem Deconinck

class RDM_Core_API Solver : public CF::Solver::CSolver {

public: // typedefs

  typedef boost::shared_ptr<Solver> Ptr;
  typedef boost::shared_ptr<Solver const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  Solver ( const std::string& name );

  /// Virtual destructor
  virtual ~Solver();

  /// Get the class name
  static std::string type_name () { return "Solver"; }

  // functions specific to the Solver component

  /// solves the PDE's
  virtual void execute();

  /// @return subcomponent for initial conditions
  InitialConditions&    initial_conditions();
  /// @return subcomponent for boundary conditions
  BoundaryConditions&   boundary_conditions();
  /// @return subcomponent for domain terms
  DomainDiscretization& domain_discretization();
  /// @return subcomponent for non linear iterative steps
  IterativeSolver&      iterative_solver();
  /// @return subcomponent for time stepping
  TimeStepping&         time_stepping();

  /// @name SIGNALS
  //@{

  //@} END SIGNALS

private: // helper functions

  void config_physics();

private: // data

  boost::shared_ptr<InitialConditions>    m_initial_conditions;    ///< subcomponent for initial conditions

  boost::shared_ptr<BoundaryConditions>   m_boundary_conditions;   ///< subcomponent for boundary conditions

  boost::shared_ptr<DomainDiscretization> m_domain_discretization; ///< subcomponent for domain terms

  boost::shared_ptr<IterativeSolver>      m_iterative_solver;      ///< subcomponent for non linear iterative steps

  boost::shared_ptr<TimeStepping>         m_time_stepping;         ///< subcomponent for time stepping

  boost::shared_ptr<CF::Solver::Actions::CSynchronizeFields> m_synchronize; ///< solution synchronization action


};

////////////////////////////////////////////////////////////////////////////////

} // Core
} // RDM
} // CF

#endif // CF_RDM_Solver_hpp
