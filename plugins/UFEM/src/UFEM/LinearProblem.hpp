// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_LinearProblem_hpp
#define CF_UFEM_LinearProblem_hpp

#include "Common/CActionDirector.hpp"
#include "Common/OptionURI.hpp"

#include "Solver/CEigenLSS.hpp"
#include "Solver/CSolver.hpp"

#include "Solver/Actions/Proto/BlockAccumulator.hpp"
#include "Solver/Actions/Proto/CProtoActionDirector.hpp"
#include "Solver/Actions/Proto/DirichletBC.hpp"
#include "Solver/Actions/Proto/SolutionVector.hpp"

#include "BoundaryConditions.hpp"
#include "LibUFEM.hpp"

namespace CF {

namespace UFEM {

/// LinearProblem for UFEM problems, allowing dynamic configuration and profiding access to
/// * Linear solver
/// * Physical model
/// * Mesh used
/// * Region to loop over
class UFEM_API LinearProblem : public Solver::Actions::Proto::CProtoActionDirector
{
public: // typedefs

  typedef boost::shared_ptr<LinearProblem> Ptr;
  typedef boost::shared_ptr<LinearProblem const> ConstPtr;

public: // functions
  
  /// Contructor
  /// @param name of the component
  LinearProblem ( const std::string& name );
  
  virtual ~LinearProblem();

  /// Get the class name
  static std::string type_name () { return "LinearProblem"; }
  
  virtual void execute();
  
  /// Return the action used for solving the system
  Common::CAction& solve_action();
  
  /// Get the component that manages boundary conditions
  BoundaryConditions& boundary_conditions();
  
private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
  
public:
  /// Proto placeholder for the system matrix
  const Solver::Actions::Proto::SystemMatrix& system_matrix;
  /// Proto placeholder for the right hand side of the system
  const Solver::Actions::Proto::SystemRHS& system_rhs;
  /// Proto placeholder for dirichlet boundary conditions
  const Solver::Actions::Proto::DirichletBC& dirichlet;
  /// Proto placeholder for the solution vector
  const Solver::Actions::Proto::SolutionVector& solution;
};

} // UFEM
} // CF


#endif // CF_UFEM_LinearProblem_hpp
