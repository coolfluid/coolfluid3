// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_ActionDirector_hpp
#define cf3_Solver_ActionDirector_hpp

#include "Common/CActionDirector.hpp"
#include "Common/FindComponents.hpp"

#include "Solver/LibSolver.hpp"

namespace cf3 {

namespace Mesh { class CMesh; }
namespace Physics { class PhysModel; }

namespace Solver {

class CSolver;
class CTime;

/////////////////////////////////////////////////////////////////////////////////////

class Solver_API ActionDirector : public common::CActionDirector {

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
  Solver::CSolver& solver();

  /// @returns the mesh this action is using
  Mesh::CMesh& mesh();

  /// @returns the physical model this action relies on
  Physics::PhysModel& physical_model();

  /// @returns the time component
  /// @deprecated CTime makes no sense in certain simulations
  ///             This will eventually be removed
  Solver::CTime& time();

  //@} END ACCESSORS


protected: // functions

  void config_regions();

protected: // data

  /// link back to the solver
  boost::weak_ptr< Solver::CSolver > m_solver;
  /// mesh where this action data resides
  boost::weak_ptr< Mesh::CMesh > m_mesh;
  /// physical model used by this action
  boost::weak_ptr< Physics::PhysModel > m_physical_model;

  /// time used by this action
  /// @todo eventually removed time from Action
  boost::weak_ptr< Solver::CTime > m_time;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Solver
} // cf3

#endif // CF3_Solver_ActionDirector_hpp
