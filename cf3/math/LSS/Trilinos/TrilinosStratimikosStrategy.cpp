// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/bind.hpp>

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
#include "ThyraMultiVector.hpp"
#include "ThyraOperator.hpp"
#include "TrilinosStratimikosStrategy.hpp"
#include "ParameterListDefaults.hpp"
#include "TrilinosVector.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file TrilinosStratimikosStrategy.hpp Drives Trilinos linear solvers through Stratimikos
  @author Bart Janssens
**/
////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

common::ComponentBuilder<TrilinosStratimikosStrategy, SolutionStrategy, LibLSS> TrilinosStratimikosStrategy_builder;

struct TrilinosStratimikosStrategy::Implementation
{
  Implementation(common::Component& self) :
    m_self(self),
    m_parameter_list(Teuchos::createParameterList())
  {
    Teko::addTekoToStratimikosBuilder(m_linear_solver_builder);
    m_linear_solver_builder.setParameterList(m_parameter_list);

    m_self.options().add("verbosity_level", 1)
      .pretty_name("Verbosity Level")
      .description("Verbosity level for the solver")
      .attach_trigger(boost::bind(&Implementation::trigger_verbosity, this))
      .mark_basic();

    m_self.options().add("compute_residual", false)
      .pretty_name("Compute Residual")
      .description("Indicate if the residual should be computed. This incurs storage of an extra vector and an extra matrix application after each solve")
      .mark_basic();

    m_self.options().add("print_settings", true)
      .pretty_name("Print Settings")
      .description("Print out the solver settings upon first solve")
      .mark_basic();
      
    m_self.options().add("settings_file", common::URI("", cf3::common::URI::Scheme::FILE))
      .supported_protocol(cf3::common::URI::Scheme::FILE)
      .pretty_name("Settings File")
      .description("If set, the settings will initially be read from this file")
      .attach_trigger(boost::bind(&Implementation::trigger_settings_file, this))
      .mark_basic();
  }

  void trigger_verbosity()
  {
    if(!m_lows_factory.is_null())
    {
      const int verb = m_self.options().option("verbosity_level").value<int>();
      m_lows_factory->setVerbLevel(static_cast<Teuchos::EVerbosityLevel>(verb));
      m_lows.reset();
    }
  }
  
  void trigger_settings_file()
  {
    const std::string settings_path = m_self.options().option("settings_file").value<common::URI>().path();
    if(settings_path.empty())
      return;
    
    if(!boost::filesystem::exists(settings_path))
    {
      CFwarn << "Settings file " << settings_path << " doesn't exist, solver parameters not changed!" << CFendl;
      return;
    }
    
    m_parameter_list = Teuchos::getParametersFromXmlFile(settings_path);
    m_linear_solver_builder.setParameterList(m_parameter_list);
    
    setup_solver();
  }

  void setup_solver()
  {
    m_lows_factory = Thyra::createLinearSolveStrategy(m_linear_solver_builder);
    const int verb = m_self.options().option("verbosity_level").value<int>();
    m_lows_factory->setVerbLevel(static_cast<Teuchos::EVerbosityLevel>(verb));
    m_lows.reset();
    m_residual_vec.reset();
    
    // Update the component tree that represents the parameters. This automatically exposes available options
    update_parameters();    
  }

  void solve()
  {
    if(is_null(m_matrix))
      throw common::SetupError(FromHere(), "Null matrix for " + m_self.uri().path());

    if(is_null(m_rhs))
      throw common::SetupError(FromHere(), "Null RHS for " + m_self.uri().path());

    if(is_null(m_solution))
      throw common::SetupError(FromHere(), "Null solution vector for " + m_self.uri().path());

    if(m_lows.is_null())
    {
      m_lows = Thyra::linearOpWithSolve(*m_lows_factory, m_matrix->thyra_operator());
      
      if(m_self.options().option("print_settings").value<bool>())
        m_parameter_list->print();
    }

    Thyra::SolveStatus<double> status = Thyra::solve<double>(*m_lows, Thyra::NOTRANS, *m_rhs->thyra_vector(m_matrix->thyra_operator()->range()), m_solution->thyra_vector(m_matrix->thyra_operator()->domain()).ptr());
    CFinfo << "Thyra::solve finished with status " << status.message << CFendl;
    if(m_self.options().option("compute_residual").value<bool>())
      CFinfo << "Solver residual: " << compute_residual() << CFendl;
  }

  Real compute_residual()
  {
    if(is_null(m_matrix))
      throw common::SetupError(FromHere(), "Null matrix for " + m_self.uri().path());

    if(is_null(m_rhs))
      throw common::SetupError(FromHere(), "Null RHS for " + m_self.uri().path());

    if(is_null(m_solution))
      throw common::SetupError(FromHere(), "Null solution vector for " + m_self.uri().path());

    if(m_lows.is_null())
      throw common::SetupError(FromHere(), "Null linear operator for " + m_self.uri().path());

    if(m_residual_vec.is_null())
    {
      m_residual_vec = m_rhs->thyra_vector(m_matrix->thyra_operator()->range())->clone_mv();
    }

    Thyra::assign(m_rhs->thyra_vector(m_matrix->thyra_operator()->range()).ptr(), *m_residual_vec);
    m_lows->apply(Thyra::NOTRANS, *m_solution->thyra_vector(m_matrix->thyra_operator()->domain()), m_residual_vec.ptr(), 1., -1.);
    std::vector<Real> residuals(m_residual_vec->domain()->dim());
    Thyra::norms_2(*m_residual_vec, Teuchos::arrayViewFromVector(residuals));
    return *std::max_element(residuals.begin(), residuals.end());
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
  Stratimikos::DefaultLinearSolverBuilder m_linear_solver_builder;

  Teuchos::RCP<Thyra::LinearOpWithSolveFactoryBase<double> > m_lows_factory;
  Teuchos::RCP<Thyra::LinearOpWithSolveBase<double> > m_lows;

  Handle<ThyraOperator const> m_matrix;
  Handle<ThyraMultiVector> m_rhs;
  Handle<ThyraMultiVector> m_solution;
  Teuchos::RCP< Thyra::MultiVectorBase<Real> > m_residual_vec;
  Handle<ParameterList> m_parameters;
};

////////////////////////////////////////////////////////////////////////////////////////////

TrilinosStratimikosStrategy::TrilinosStratimikosStrategy(const std::string& name) :
  SolutionStrategy(name),
  m_implementation(new Implementation(*this))
{
  set_default_parameters("cf3.math.LSS.BelosGMRESParameters");
  m_implementation->update_parameters();

  common::Core::instance().event_handler().connect_to_event("trilinos_parameters_changed", this, &TrilinosStratimikosStrategy::on_parameters_changed_event);
}

TrilinosStratimikosStrategy::~TrilinosStratimikosStrategy()
{
}


void TrilinosStratimikosStrategy::set_matrix(const Handle< Matrix >& matrix)
{
  m_implementation->m_matrix = Handle<ThyraOperator>(matrix);
  m_implementation->setup_solver();
}

void TrilinosStratimikosStrategy::set_rhs(const Handle< Vector >& rhs)
{
  m_implementation->m_rhs = Handle<ThyraMultiVector>(rhs);
}

void TrilinosStratimikosStrategy::set_solution(const Handle< Vector >& solution)
{
  m_implementation->m_solution = Handle<ThyraMultiVector>(solution);
}

void TrilinosStratimikosStrategy::solve()
{
  m_implementation->solve();
}

Real TrilinosStratimikosStrategy::compute_residual()
{
  return m_implementation->compute_residual();
}


void TrilinosStratimikosStrategy::set_default_parameters(const string& builder_name)
{
  common::build_component_abstract_type<ParameterListDefaults>(builder_name, "DefaultParams")->set_parameters(*m_implementation->m_parameter_list);
  m_implementation->update_parameters();
}

void TrilinosStratimikosStrategy::on_parameters_changed_event(common::SignalArgs& args)
{
  common::XML::SignalOptions options(args);
  const common::URI parameters_uri = options.value<common::URI>("parameters_uri");

  if(boost::starts_with(parameters_uri.path(), uri().path()))
  {
    CFdebug << "Acting on trilinos_parameters_changed event from paramater list " << parameters_uri.string() << CFendl;
    m_implementation->setup_solver();
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
