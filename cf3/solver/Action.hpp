// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_Action_hpp
#define cf3_solver_Action_hpp

#include "common/Action.hpp"

#include "solver/LibSolver.hpp"

namespace cf3 {
namespace common { template <typename T> struct ComponentIterator; }
namespace mesh { class Region; class Mesh; }
namespace physics { class PhysModel; }
namespace solver {

class Solver;
class Time;

////////////////////////////////////////////////////////////////////////////////////////////

class solver_API Action : public common::Action {

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
  solver::Solver& solver();

  /// @returns the mesh this action is using
  mesh::Mesh& mesh();

  /// @returns the physical model this action relies on
  physics::PhysModel& physical_model();

  /// @returns the regions this action is operating on
  const std::vector< Handle< mesh::Region > >& regions() const;

  //@} END ACCESSORS

protected: // functions

  void config_regions();

protected: // data

  /// link back to the solver
  Handle< solver::Solver > m_solver;
  /// mesh where this action data resides
  Handle< mesh::Mesh > m_mesh;
  /// physical model used by this action
  Handle< physics::PhysModel > m_physical_model;

  /// regions of the mesh to loop over
  std::vector< Handle< mesh::Region > > m_loop_regions;

  /// Called after the regions have been set
  virtual void on_regions_set();

};

////////////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

#endif // cf3_solver_Action_hpp
