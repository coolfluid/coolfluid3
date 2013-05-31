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

#include "BelosLinearProblem.hpp"
#include "BelosEpetraAdapter.hpp"
#include "BelosThyraAdapter.hpp"
#include "BelosRCGSolMgr.hpp"

#include "ml_include.h"
#include "ml_MultiLevelPreconditioner.h"

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
#include "Thyra_MLPreconditionerFactory.hpp"

#include "Stratimikos_DefaultLinearSolverBuilder.hpp"

#include "common/Builder.hpp"
#include "common/EventHandler.hpp"
#include "common/OptionList.hpp"
#include <common/Table.hpp>
#include <common/List.hpp>

#include "ParameterList.hpp"
#include "TrilinosVector.hpp"
#include "TrilinosCrsMatrix.hpp"
#include "RCGStrategy.hpp"

namespace cf3 {
namespace math {
namespace LSS {

common::ComponentBuilder<RCGStrategy, SolutionStrategy, LibLSS> RCGStrategy_builder;

struct RCGStrategy::Implementation
{
  typedef Epetra_MultiVector MV;
  typedef Epetra_Operator OP;
  typedef Belos::MultiVecTraits< Real, MV > MVT;

  Implementation(common::Component& self) :
    m_self(self),
    m_ml_parameter_list(Teuchos::createParameterList()),
    m_solver_parameter_list(Teuchos::createParameterList())
  {
    // ML default parameters
    ML_Epetra::SetDefaults("SA", *m_ml_parameter_list);
    m_ml_parameter_list->set("ML output", 10);
    m_ml_parameter_list->set("max levels",3);
    m_ml_parameter_list->set("aggregation: type", "METIS");
    m_ml_parameter_list->set("aggregation: nodes per aggregate", 16);
    m_ml_parameter_list->set("smoother: type","Chebyshev");
    m_ml_parameter_list->set("smoother: sweeps",2);
    m_ml_parameter_list->set("smoother: pre or post", "both");
    
    Amesos amesos;
    if(amesos.Query("Amesos_Mumps"))
    {
      m_ml_parameter_list->set("coarse: type","Amesos-MUMPS");
    }
    else
    {
      m_ml_parameter_list->set("coarse: type","Amesos-KLU");
    }
    
    // Default solver parameters
    m_solver_parameter_list->set( "Verbosity", Belos::TimingDetails | Belos::FinalSummary );
    m_solver_parameter_list->set( "Block Size", 1 );
    m_solver_parameter_list->set( "Num Blocks", 400 );
    m_solver_parameter_list->set( "Maximum Iterations", 500 );
    m_solver_parameter_list->set( "Convergence Tolerance", 1.0e-4 );
    m_solver_parameter_list->set( "Num Recycled Blocks", 300 );

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

    // Create the problem
    m_problem = Teuchos::rcp( new Belos::LinearProblem<Real,MV,OP>(m_matrix->epetra_matrix(), m_solution->epetra_vector(), m_rhs->epetra_vector()) );

    Handle< common::Table<Real> const > coordinates = m_self.options().value< Handle< common::Table<Real> > >("coordinates");
    Handle< common::List<Uint> const > used_nodes = m_self.options().value< Handle< common::List<Uint> > >("used_nodes");

    if(is_not_null(coordinates) && is_not_null(used_nodes))
    {
      const int dim = coordinates->row_size();
      const Uint nb_nodes = used_nodes->size();
      for(Uint i = 0; i != dim; ++i)
      {
        m_coords[i].resize(nb_nodes);
      }

      for(Uint inode = 0; inode != nb_nodes; ++inode)
      {
        // Get local matrix index
        const int mat_idx = m_matrix->matrix_index(inode, 0);
        cf3_assert(mat_idx < nb_nodes);
        // Store coord at location
        const common::Table<Real>::ConstRow r = (*coordinates)[(*used_nodes)[inode]];
        for(int i = 0; i != dim; ++i)
        {
          m_coords[i][mat_idx] = r[i];
        }
      }

      m_ml_parameter_list->set("repartition: enable",1);
      m_ml_parameter_list->set("repartition: max min ratio",1.3);
      m_ml_parameter_list->set("repartition: min per proc",500);
      m_ml_parameter_list->set("repartition: partitioner","Zoltan");
      m_ml_parameter_list->set("repartition: Zoltan dimensions",dim);
      
      m_ml_parameter_list->set("x-coordinates", &m_coords[0][0]);
      m_ml_parameter_list->set("y-coordinates", &m_coords[1][0]);
      if(dim == 3)
        m_ml_parameter_list->set("z-coordinates", &m_coords[2][0]);
    }
    
    // Set the preconditioner
    m_ml_prec = Teuchos::rcp(new ML_Epetra::MultiLevelPreconditioner(*m_matrix->epetra_matrix(), *m_ml_parameter_list, true));
    Teuchos::RCP<Belos::EpetraPrecOp> belos_prec = Teuchos::rcp(new Belos::EpetraPrecOp(m_ml_prec));
    m_problem->setLeftPrec(belos_prec);

    m_solver = Teuchos::rcp(new Belos::RCGSolMgr<double,MV,OP>(m_problem, m_solver_parameter_list));

    return 0;
  }

  void solve()
  {
    if(is_null(m_solver.get()))
    {
      setup_solver();
    }

    if(!m_problem->setProblem())
      throw common::SetupError(FromHere(), "Error setting up Belos problem");

    m_solver->solve();
  }

  Real compute_residual()
  {
    return -1.;
  }

  void update_parameters()
  {
    if(is_not_null(m_ml_parameters))
      m_self.remove_component("MLParameters");

    m_ml_parameters = m_self.create_component<ParameterList>("MLParameters");
    m_ml_parameters->mark_basic();
    m_ml_parameters->set_parameter_list(*m_ml_parameter_list);

    if(is_not_null(m_solver_parameters))
      m_self.remove_component("SolverParameters");

    m_solver_parameters = m_self.create_component<ParameterList>("SolverParameters");
    m_solver_parameters->mark_basic();
    m_solver_parameters->set_parameter_list(*m_solver_parameter_list);
  }

  void reset_solver()
  {
    m_solver.reset();
    m_problem.reset();
    m_ml_prec.reset();
  }

  common::Component& m_self;
  Teuchos::RCP<Teuchos::ParameterList> m_ml_parameter_list;
  Teuchos::RCP<Teuchos::ParameterList> m_solver_parameter_list;
  Teuchos::RCP<ML_Epetra::MultiLevelPreconditioner> m_ml_prec;
  Teuchos::RCP< Belos::LinearProblem<Real,MV,OP> > m_problem;
  Teuchos::RCP< Belos::RCGSolMgr<double,MV,OP> > m_solver;

  Handle<TrilinosCrsMatrix> m_matrix;
  Handle<TrilinosVector> m_rhs;
  Handle<TrilinosVector> m_solution;
  Handle<ParameterList> m_ml_parameters;
  Handle<ParameterList> m_solver_parameters;

  std::vector<Real> m_coords[3];
};

RCGStrategy::RCGStrategy(const string& name) :
  SolutionStrategy(name),
  m_implementation(new Implementation(*this))
{
  options().add("coordinates", Handle< common::Table<Real> >())
    .pretty_name("Coordinates")
    .description("Coordinates of the mesh associated with the matrix")
    .attach_trigger(boost::bind(&Implementation::reset_solver, m_implementation.get()));
    
  options().add("used_nodes", Handle< common::List<Uint> >())
    .pretty_name("Used Nodes")
    .description("List of used nodes for this matrix")
    .attach_trigger(boost::bind(&Implementation::reset_solver, m_implementation.get()));
}

RCGStrategy::~RCGStrategy()
{
}

Real RCGStrategy::compute_residual()
{
  return m_implementation->compute_residual();
}

void RCGStrategy::set_rhs(const Handle< Vector >& rhs)
{
  m_implementation->m_rhs = Handle<TrilinosVector>(rhs);
}

void RCGStrategy::set_solution(const Handle< Vector >& solution)
{
  m_implementation->m_solution = Handle<TrilinosVector>(solution);
}

void RCGStrategy::set_matrix(const Handle< Matrix >& matrix)
{
  m_implementation->m_matrix = Handle<TrilinosCrsMatrix>(matrix);
}

void RCGStrategy::solve()
{
  m_implementation->solve();
}

void RCGStrategy::on_parameters_changed_event(common::SignalArgs& args)
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
