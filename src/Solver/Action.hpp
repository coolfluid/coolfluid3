// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_Action_hpp
#define cf3_Solver_Action_hpp

#include "Common/CAction.hpp"
#include "Common/FindComponents.hpp"

#include "Solver/LibSolver.hpp"

namespace cf3 {

namespace Mesh { class CRegion; class CMesh; }
namespace Physics { class PhysModel; }
namespace Solver {

class CSolver;
class CTime;

////////////////////////////////////////////////////////////////////////////////////////////

class Solver_API Action : public common::CAction {

public: // typedefs

  /// provider
  typedef boost::shared_ptr< Action > Ptr;
  typedef boost::shared_ptr< Action const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  Action ( const std::string& name );

  /// Virtual destructor
  virtual ~Action();

  /// Get the class name
  static std::string type_name () { return "Action"; }

  /// @name ACCESSORS
  //@{

  /// @returns the solver this action is part of
  Solver::CSolver& solver();

  /// @returns the mesh this action is using
  Mesh::CMesh& mesh();

  /// @returns the physical model this action relies on
  Physics::PhysModel& physical_model();

  /// @returns the regions this action is operating on
  common::ComponentIteratorRange<Mesh::CRegion> regions();

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

  /// regions of the mesh to loop over
  std::vector< boost::shared_ptr< Mesh::CRegion > > m_loop_regions;

};

////////////////////////////////////////////////////////////////////////////////////////////

} // Solver
} // cf3

#endif // CF3_Solver_Action_hpp
