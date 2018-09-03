// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/bind.hpp>

#include "Amesos.h"
#include "Amesos_BaseSolver.h"

#include "Epetra_LinearProblem.h"

#include "Teuchos_ConfigDefs.hpp"
#include "Teuchos_RCP.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"

#include "common/Builder.hpp"
#include "common/EventHandler.hpp"
#include "common/OptionList.hpp"

#include "ParameterList.hpp"
#include "TrilinosVector.hpp"
#include "TrilinosCrsMatrix.hpp"
#include "DirectStrategy.hpp"

namespace cf3 {
namespace math {
namespace LSS {

common::ComponentBuilder<DirectStrategy, SolutionStrategy, LibLSS> DirectStrategy_builder;

struct DirectStrategy::Implementation
{
  Implementation(common::Component& self) :
    m_self(self),
    m_solver_parameter_list(Teuchos::createParameterList()),
    m_solver_type("Amesos_Klu"),
    m_interval(0u),
    m_count(0u)
  {
    // Default solver parameters
    m_solver_parameter_list->set("PrintTiming",true);
    m_solver_parameter_list->set("PrintStatus",true);
    m_solver_parameter_list->set("MaxProcs",common::PE::Comm::instance().size());

    self.options().add("solver_type", m_solver_type)
      .pretty_name("Solver Type")
      .description("Solver type for Amesos factory")
      .link_to(&m_solver_type)
      .attach_trigger(boost::bind(&Implementation::reset_solver, this))
      .mark_basic();
      
    self.options().add("interval", m_interval)
      .pretty_name("Interval")
      .description("Interval between recalculation of the factorization")
      .link_to(&m_interval)
      .mark_basic();
    
    update_parameters();
  }

  int setup_solver()
  {
    if(is_null(m_matrix))
      throw common::SetupError(FromHere(), "Null matrix for " + m_self.uri().path());

    if(is_null(m_rhs))
      throw common::SetupError(FromHere(), "Null RHS for " + m_self.uri().path());

    if(is_null(m_solution))
      throw common::SetupError(FromHere(), "Null solution vector for " + m_self.uri().path());

    if(!m_factory.Query(m_solver_type))
      throw common::SetupError(FromHere(), "Solver " + m_solver_type + " is not available.");
    
    m_problem.SetOperator(m_matrix->epetra_matrix().get());
    m_problem.SetLHS(m_solution->epetra_vector().get());
    m_problem.SetRHS(m_rhs->epetra_vector().get());
    
    // Create the solver
    m_solver.reset(m_factory.Create(m_solver_type, m_problem));
    
    // Factorize the matrix
    CFdebug << "Directstrategy: factoring matrix" << CFendl;
    m_solver->SymbolicFactorization();
    m_solver->NumericFactorization();

    return 0;
  }

  void solve()
  {
    if(m_interval != 0 && (m_count % m_interval) == 0)
      reset_solver();
    
    if(is_null(m_solver.get()))
    {
      setup_solver();
    }

    m_solver->Solve();
    
    ++m_count;
  }

  Real compute_residual()
  {
    return -1.;
  }

  void update_parameters()
  {
    if(is_not_null(m_solver_parameters))
      m_self.remove_component("SolverParameters");

    m_solver_parameters = m_self.create_component<ParameterList>("SolverParameters");
    m_solver_parameters->mark_basic();
    m_solver_parameters->set_parameter_list(*m_solver_parameter_list);
  }

  void reset_solver()
  {
    m_solver.reset();
  }

  common::Component& m_self;
  Teuchos::RCP<Teuchos::ParameterList> m_solver_parameter_list;

  Handle<TrilinosCrsMatrix> m_matrix;
  Handle<TrilinosVector> m_rhs;
  Handle<TrilinosVector> m_solution;
  Handle<ParameterList> m_solver_parameters;
  
  Amesos m_factory;
  
  std::auto_ptr<Amesos_BaseSolver> m_solver;
  
  std::string m_solver_type;
  Epetra_LinearProblem m_problem;
  
  Uint m_interval;
  Uint m_count;
};

DirectStrategy::DirectStrategy(const std::string& name) :
  CoordinatesStrategy(name),
  m_implementation(new Implementation(*this))
{
}

DirectStrategy::~DirectStrategy()
{
}

Real DirectStrategy::compute_residual()
{
  return m_implementation->compute_residual();
}

void DirectStrategy::set_rhs(const Handle< Vector >& rhs)
{
  m_implementation->m_rhs = Handle<TrilinosVector>(rhs);
}

void DirectStrategy::set_solution(const Handle< Vector >& solution)
{
  m_implementation->m_solution = Handle<TrilinosVector>(solution);
}

void DirectStrategy::set_matrix(const Handle< Matrix >& matrix)
{
  m_implementation->m_matrix = Handle<TrilinosCrsMatrix>(matrix);
}

void DirectStrategy::solve()
{
  m_implementation->solve();
}

void DirectStrategy::on_parameters_changed_event(common::SignalArgs& args)
{
  common::XML::SignalOptions options(args);
  const common::URI parameters_uri = options.value<common::URI>("parameters_uri");

  if(boost::starts_with(parameters_uri.path(), uri().path()))
  {
    CFdebug << "Acting on trilinos_parameters_changed event from paramater list " << parameters_uri.string() << CFendl;
    m_implementation->reset_solver();
  }
  else
  {
    CFdebug << "Ignoring trilinos_parameters_changed event from paramater list " << parameters_uri.string() << CFendl;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace math
} // namespace cf3
