// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Signal.hpp"
#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/EventHandler.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionComponent.hpp"
#include "common/ActionDirector.hpp"
#include "common/FindComponents.hpp"
#include "common/Group.hpp"

#include "math/VariablesDescriptor.hpp"

#include "solver/Time.hpp"
#include "solver/Solver.hpp"

#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

#include "sdm/ExplicitRungeKuttaLowStorage3.hpp"
#include "sdm/Tags.hpp"
#include "sdm/SDSolver.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::mesh;

namespace cf3 {
namespace sdm {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ExplicitRungeKuttaLowStorage3, common::Action, LibSDM > ExplicitRungeKuttaLowStorage3_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ExplicitRungeKuttaLowStorage3::ExplicitRungeKuttaLowStorage3 ( const std::string& name ) :
  IterativeSolver(name)
{
  options().add("order", 1u)
      .description("Order of the Runge-Kutta integration")
      .pretty_name("RK order");

  options().add("nb_stages", 1u).mark_basic()
      .description("Number of stages of the Runge-Kutta integration")
      .pretty_name("RK stages")
      .attach_trigger( boost::bind( &ExplicitRungeKuttaLowStorage3::config_nb_stages , this ) );

  std::vector<Real> dummy(4);
  options().add("delta", dummy)
      .description("RK coefficients delta (length = nb_stages)");

  options().add("gamma1", dummy)
      .description("RK coefficients gamma1 (length = nb_stages)");

  options().add("gamma2", dummy)
      .description("RK coefficients gamma2 (length = nb_stages)");

  options().add("gamma3", dummy)
      .description("RK coefficients gamma3 (length = nb_stages)");

  options().add("beta", dummy)
      .description("RK coefficients beta, lowstorage: vector instead of matrix (stage+1,stage)  (length = nb_stages)");

  options().add("c", dummy)
      .description("coefficients c from butcher tableau (length = nb_stages)");

  config_nb_stages();
}

///////////////////////////////////////////////////////////////////////////////////////

void ExplicitRungeKuttaLowStorage3::config_nb_stages()
{
  const Uint nb_stages = options().value<Uint>("nb_stages");

  // Here can be some default coefficients set for the given number of stages
}

///////////////////////////////////////////////////////////////////////////////////////

void ExplicitRungeKuttaLowStorage3::link_fields()
{
  IterativeSolver::link_fields();

  if ( is_null(m_solution_backup) )  // backup not created --> create field
  {
    if (Handle< Component > found_solution_backup = solver().field_manager().get_child( "solution_backup" ))
    {
      m_solution_backup = Handle<Field>( follow_link(found_solution_backup) );
    }
    else if ( Handle< Component > found_solution_backup = m_solution->dict().get_child( "solution_backup" ) )
    {
      solver().field_manager().create_component<Link>("solution_backup")->link_to(*found_solution_backup);
      m_solution_backup = found_solution_backup->handle<Field>();
    }
    else
    {
      m_solution_backup = m_solution->dict().create_field("solution_backup", m_solution->descriptor().description()).handle<Field>();
      m_solution_backup->descriptor().prefix_variable_names("backup_");
      solver().field_manager().create_component<Link>("solution_backup")->link_to(*m_solution_backup);
    }
  }

  if ( is_null(m_S2) )  // register not created --> create field
  {
    if (Handle< Component > found_register = solver().field_manager().get_child( "solution_register" ))
    {
      m_S2 = Handle<Field>( follow_link(found_register) );
    }
    else if ( Handle< Component > found_register = m_solution->dict().get_child( "solution_register" ) )
    {
      solver().field_manager().create_component<Link>("solution_register")->link_to(*found_register);
      m_S2 = found_register->handle<Field>();
    }
    else
    {
      m_S2 = m_solution->dict().create_field("solution_register", m_solution->descriptor().description()).handle<Field>();
      m_S2->descriptor().prefix_variable_names("S2_");
      solver().field_manager().create_component<Link>("solution_register")->link_to(*m_S2);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void ExplicitRungeKuttaLowStorage3::execute()
{
  configure_option_recursively( "iterator", handle<Component>() );

  link_fields();

  int convergence_failed = false;
  const Uint nb_stages = options().value<Uint>("nb_stages");
  std::vector<Real> delta_vec = options().value< std::vector<Real> >("delta");
  std::vector<Real> gamma1_vec = options().value< std::vector<Real> >("gamma1");
  std::vector<Real> gamma2_vec = options().value< std::vector<Real> >("gamma2");
  std::vector<Real> gamma3_vec = options().value< std::vector<Real> >("gamma3");
  std::vector<Real> beta_vec  = options().value< std::vector<Real> >("beta");
  std::vector<Real> c_vec     = options().value< std::vector<Real> >("c");

  if (c_vec[0] != 0)
    throw NotSupported(FromHere(),
                       "It is assumed that c[0]=0 (here c[0]="+to_str(c_vec[0])+"),\n"
                       "so that the time-step can be computed in the first stage");

  Real c;
  Real delta;
  Real gamma1;
  Real gamma2;
  Real gamma3;
  Real beta;

  Field& S1 = *m_solution;
  Field& S2 = *m_S2;
  Field& S3 = *m_solution_backup;
  Field& R  = *m_residual;
  Field& H  = *m_update_coeff;

  if (is_null(m_time))        throw SetupError(FromHere(), "Time was not set");
  Time& time = *m_time;

  S2 = 0.;
  S3 = S1;

  const Real T0 = time.current_time();
  Real dt = 0;

  for (Uint stage=0; stage<nb_stages; ++stage)
  {
    // Just some shortcuts for efficiency
    delta  = delta_vec[stage];
    gamma1 = gamma1_vec[stage];
    gamma2 = gamma2_vec[stage];
    gamma3 = gamma3_vec[stage];
    beta   = beta_vec[stage];
    c      = c_vec[stage];

    // Set time and iteration for this stage
    properties().property("iteration") = stage+1;

    time.current_time() = T0 + c * dt;

    // Do actual computations in pre_update
    try
    {
      pre_update().execute();
    }
    catch (const common::FailedToConverge& exception)
    {
      convergence_failed = true;
    }
    PE::Comm::instance().all_reduce(PE::max(),&convergence_failed,1,&convergence_failed);
    if (convergence_failed)
      throw (common::FailedToConverge(FromHere(),""));


    // now assigned in pre-update
    // - R

    // Only in case of the first stage, compute the time-step (= update coefficient)
    if (stage == 0)
      solver().handle<SDSolver>()->actions().get_child("compute_update_coefficient")->handle<common::Action>()->execute();
    // now assigned:

    // - H
    // - time.dt()

    /// // Use convention indexes start at 1
    /// S1 := U(t=n)   S2 := 0   S3 := U(t=n)
    /// for i = 2:m+1 do
    ///     S2 := S2 + delta(i-1)*S1
    ///     S1 := gamma(i,1)*S1 + gamma(i,2)*S2 + gamma(i,3)*S3 + beta(i,i-1)*dt*F(S1)
    /// end
    /// U(t=n+1) = S1
    /// // for error_estimate, use:
    ///     S2 := 1/sum(delta) * (S2 + delta(m+1)*S1 + delta(m+2)*S3

    boost_foreach(const Handle<Entities>& elements_handle, m_solution->entities_range())
    {
      Entities& elements = *elements_handle;
      const Connectivity& space_connectivity = m_solution->space(elements).connectivity();

      for (Uint e=0; e<elements.size(); ++e)
      {
        boost_foreach(const Uint state, space_connectivity[e])
        {
          for (Uint var=0; var<m_solution->row_size(); ++var)
          {
            S2[state][var] = S2[state][var] + delta*S1[state][var];
            S1[state][var] =   gamma1*S1[state][var]
                             + gamma2*S2[state][var]
                             + gamma3*S3[state][var]
                             + beta*H[state][0]*R[state][var];
          }
        }
      }
    }

    // U (=S1) has now been updated

    // Do post-processing per stage after update
    post_update().execute();
    m_solution->synchronize();

    // Prepare for next stage
    if (stage == 0)
    {
      dt = time.dt();
    }
    else
    {
      time.dt() = dt;
    }
    time.current_time() = T0;
    // raise signal that iteration is done
    raise_iteration_done();
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void ExplicitRungeKuttaLowStorage3::raise_iteration_done()
{
  SignalOptions opts;
  const Uint iter = properties().value<Uint>("iteration");
  opts.add( "iteration", iter );
  SignalFrame frame = opts.create_frame("iteration_done", uri(), URI());

  common::Core::instance().event_handler().raise_event( "iteration_done", frame);
}

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3
