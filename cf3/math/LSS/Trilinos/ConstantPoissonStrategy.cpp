// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/bind.hpp>

#include "ml_include.h"
#include "ml_MultiLevelPreconditioner.h"
#include "AztecOO.h"

#include "Epetra_LinearProblem.h"

#include "Teuchos_ConfigDefs.hpp"
#include "Teuchos_RCP.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"

#include "Teko_StratimikosFactory.hpp"

#include "Thyra_EpetraLinearOp.hpp"
#include "Thyra_EpetraThyraWrappers.hpp"
#include "Thyra_LinearOpWithSolveBase.hpp"
#include "Thyra_VectorBase.hpp"
#include "Thyra_MultiVectorStdOps.hpp"

#include "Stratimikos_DefaultLinearSolverBuilder.hpp"

#include "common/Builder.hpp"
#include "common/EventHandler.hpp"
#include "common/OptionList.hpp"

#include "ParameterList.hpp"
#include "TrilinosVector.hpp"
#include "TrilinosCrsMatrix.hpp"
#include "ConstantPoissonStrategy.hpp"

namespace cf3 {
namespace math {
namespace LSS {

common::ComponentBuilder<ConstantPoissonStrategy, SolutionStrategy, LibLSS> ConstantPoissonStrategy_builder;

struct ConstantPoissonStrategy::Implementation
{
  Implementation(common::Component& self) :
    m_self(self),
    m_parameter_list(Teuchos::createParameterList())
  {
    ML_Epetra::SetDefaults("SA", *m_parameter_list);
    m_parameter_list->set("ML output", 10);
    // maximum number of levels
    m_parameter_list->set("max levels",10);

    // use Uncoupled scheme to create the aggregate
    m_parameter_list->set("aggregation: type", "MIS");

    m_parameter_list->set("smoother: type","Chebyshev");
    m_parameter_list->set("smoother: sweeps",3);

    // use both pre and post smoothing
    m_parameter_list->set("smoother: pre or post", "both");

    m_parameter_list->set("coarse: type","Amesos-KLU");

    update_parameters();
  }

  void setup_solver()
  {
    if(is_null(m_matrix))
      throw common::SetupError(FromHere(), "Null matrix for " + m_self.uri().path());

    if(is_null(m_rhs))
      throw common::SetupError(FromHere(), "Null RHS for " + m_self.uri().path());

    if(is_null(m_solution))
      throw common::SetupError(FromHere(), "Null solution vector for " + m_self.uri().path());

    m_parameter_list->print();

    m_ml_prec.reset(new ML_Epetra::MultiLevelPreconditioner(*m_matrix->epetra_matrix(), *m_parameter_list, true));

    m_problem.reset(new Epetra_LinearProblem(m_matrix->epetra_matrix().get(), m_solution->epetra_vector().get(), m_rhs->epetra_vector().get()));
    m_solver.reset(new AztecOO(*m_problem));
    m_solver->SetPrecOperator(m_ml_prec.get());
    m_solver->SetAztecOption(AZ_solver, AZ_cg);
    m_solver->SetAztecOption(AZ_output, 1);
  }

  void solve()
  {
    if(is_null(m_solver.get()))
    {
      setup_solver();
    }

    m_solver->Iterate(500, 1e-6);
  }

  Real compute_residual()
  {
    return -1.;
  }

  void update_parameters()
  {
    if(is_not_null(m_parameters))
      m_self.remove_component("Parameters");

    m_parameters = m_self.create_component<ParameterList>("Parameters");
    m_parameters->mark_basic();
    m_parameters->set_parameter_list(*m_parameter_list);
  }

  common::Component& m_self;
  Teuchos::RCP<Teuchos::ParameterList> m_parameter_list;
  boost::scoped_ptr<ML_Epetra::MultiLevelPreconditioner> m_ml_prec;
  boost::scoped_ptr<Epetra_LinearProblem> m_problem;
  boost::scoped_ptr<AztecOO> m_solver;

  Handle<TrilinosCrsMatrix> m_matrix;
  Handle<TrilinosVector> m_rhs;
  Handle<TrilinosVector> m_solution;
  Handle<ParameterList> m_parameters;
};

ConstantPoissonStrategy::ConstantPoissonStrategy(const string& name) :
  SolutionStrategy(name),
  m_implementation(new Implementation(*this))
{
}

ConstantPoissonStrategy::~ConstantPoissonStrategy()
{
}

Real ConstantPoissonStrategy::compute_residual()
{
  return m_implementation->compute_residual();
}

void ConstantPoissonStrategy::set_rhs(const Handle< Vector >& rhs)
{
  m_implementation->m_rhs = Handle<TrilinosVector>(rhs);
}

void ConstantPoissonStrategy::set_solution(const Handle< Vector >& solution)
{
  m_implementation->m_solution = Handle<TrilinosVector>(solution);
}

void ConstantPoissonStrategy::set_matrix(const Handle< Matrix >& matrix)
{
  m_implementation->m_matrix = Handle<TrilinosCrsMatrix>(matrix);
}

void ConstantPoissonStrategy::solve()
{
  m_implementation->solve();
}

void ConstantPoissonStrategy::on_parameters_changed_event(common::SignalArgs& args)
{
  common::XML::SignalOptions options(args);
  const common::URI parameters_uri = options.value<common::URI>("parameters_uri");

  if(boost::starts_with(parameters_uri.path(), uri().path()))
  {
    CFdebug << "Acting on trilinos_parameters_changed event from paramater list " << parameters_uri.string() << CFendl;
    m_implementation->m_solver.reset();
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
