// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_LinearSolver_hpp
#define cf3_UFEM_LinearSolver_hpp

#include "common/ActionDirector.hpp"
#include "common/OptionURI.hpp"

#include "solver/CSimpleSolver.hpp"

#include "solver/actions/Proto/BlockAccumulator.hpp"
#include "solver/actions/Proto/DirichletBC.hpp"
#include "solver/actions/Proto/SolutionVector.hpp"

#include "BoundaryConditions.hpp"
#include "LibUFEM.hpp"

namespace cf3 {

namespace UFEM {

/// LinearSolver for UFEM problems, allowing dynamic configuration and providing access to
/// * Linear system solver
/// * Physical model
/// * Mesh used
/// * Region to loop over
class UFEM_API LinearSolver : public solver::CSimpleSolver
{
public: // functions

  /// Contructor
  /// @param name of the component
  LinearSolver ( const std::string& name );

  virtual ~LinearSolver();

  /// Get the class name
  static std::string type_name () { return "LinearSolver"; }

  virtual void execute();

  virtual void mesh_loaded(mesh::Mesh& mesh);
  virtual void mesh_changed(mesh::Mesh& mesh);

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;

  /// Trigerred when the LSS is set
  void trigger_lss();

public:
  /// Proto placeholder for the system matrix
  const solver::actions::Proto::SystemMatrix& system_matrix;
  /// Proto placeholder for the right hand side of the system
  const solver::actions::Proto::SystemRHS& system_rhs;
  /// Proto placeholder for dirichlet boundary conditions
  const solver::actions::Proto::DirichletBC& dirichlet;
  /// Proto placeholder for the solution vector
  const solver::actions::Proto::SolutionVector& solution;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_LinearSolver_hpp
