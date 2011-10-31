// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_ActionDirector_hpp
#define cf3_solver_ActionDirector_hpp

#include "common/ActionDirector.hpp"

#include "solver/LibSolver.hpp"

namespace cf3 {

namespace mesh { class Mesh; }
namespace physics { class PhysModel; }

namespace solver {

class CSolver;
class CTime;

/////////////////////////////////////////////////////////////////////////////////////

class solver_API ActionDirector : public common::ActionDirector {

public: // typedefs

  /// provider
  typedef boost::shared_ptr< ActionDirector > Ptr;
  typedef boost::shared_ptr< ActionDirector const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  ActionDirector ( const std::string& name );

  /// Virtual destructor
  virtual ~ActionDirector();

  /// Get the class name
  static std::string type_name () { return "ActionDirector"; }

  /// @name ACCESSORS
  //@{

  /// @returns the solver this action is part of
  solver::CSolver& solver();

  /// @returns the mesh this action is using
  mesh::Mesh& mesh();

  /// @returns the physical model this action relies on
  physics::PhysModel& physical_model();

  /// @returns the time component
  /// @deprecated CTime makes no sense in certain simulations
  ///             This will eventually be removed
  solver::CTime& time();

  //@} END ACCESSORS


protected: // functions

  void config_regions();

protected: // data

  /// link back to the solver
  boost::weak_ptr< solver::CSolver > m_solver;
  /// mesh where this action data resides
  boost::weak_ptr< mesh::Mesh > m_mesh;
  /// physical model used by this action
  boost::weak_ptr< physics::PhysModel > m_physical_model;

  /// time used by this action
  /// @todo eventually removed time from Action
  boost::weak_ptr< solver::CTime > m_time;

};

/////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

#endif // cf3_solver_ActionDirector_hpp
