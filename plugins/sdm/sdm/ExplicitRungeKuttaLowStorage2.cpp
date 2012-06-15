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

#include "sdm/ExplicitRungeKuttaLowStorage2.hpp"
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

common::ComponentBuilder < ExplicitRungeKuttaLowStorage2, common::Action, LibSDM > ExplicitRungeKuttaLowStorage2_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ExplicitRungeKuttaLowStorage2::ExplicitRungeKuttaLowStorage2 ( const std::string& name ) :
  IterativeSolver(name)
{
  options().add("order", 1u)
      .description("Order of the Runge-Kutta integration")
      .pretty_name("RK order");

  options().add("nb_stages", 1u).mark_basic()
      .description("Number of stages of the Runge-Kutta integration")
      .pretty_name("RK stages")
      .attach_trigger( boost::bind( &ExplicitRungeKuttaLowStorage2::config_nb_stages , this ) );

  std::vector<Real> dummy(4);
  options().add("alpha", dummy)
      .description("RK coefficients alpha")
      .pretty_name("alpha")
      .link_to(&m_alpha);

  options().add("beta", dummy)
      .description("RK coefficients beta")
      .pretty_name("beta")
      .link_to(&m_beta);

  options().add("gamma", dummy)
      .description("RK coefficients gamma")
      .pretty_name("gamma")
      .link_to(&m_gamma);

  config_nb_stages();
}

///////////////////////////////////////////////////////////////////////////////////////

void ExplicitRungeKuttaLowStorage2::config_nb_stages()
{
  const Uint nb_stages = options().value<Uint>("nb_stages");

  std::vector<Real> alpha(nb_stages,0);
  std::vector<Real> beta(nb_stages,0);
  std::vector<Real> gamma(nb_stages,0);

  // Set defaults Values
  switch (nb_stages)
  {
    case 1: // Simple Forward Euler
      options().set("order",1u);
      alpha[0] = 0.0;
      beta[0] = 1.0;
      gamma[0] = 0.0;
      break;

    case 2: // R-K 2
      options().set("order",2u);

      alpha[0] = 0.0;
      alpha[1] = 0.0;

      beta[0] = 0.5;
      beta[1] = 1.0;

      gamma[0] = 0.0;
      gamma[1] = 0.5;
      break;

    case 3:  // 3rd order TVD R-K scheme
      options().set("order",3u);

      alpha[0] = 0.0;
      alpha[1] = 1.0/4.0;
      alpha[2] = 2.0/3.0;

      beta[0] = 1.0;
      beta[1] = 1.0/4.0;
      beta[2] = 2.0/3.0;

      gamma[0] = 0.0;
      gamma[1] = 0.5;
      gamma[2] = 1.0;
      break;

    case 4:    // R-K 4
      options().set("order",4u);

      alpha[0] = 0.0;
      alpha[1] = 0.0;
      alpha[2] = 0.0;
      alpha[3] = 0.0;

      beta [0] = 1.0/4.0;
      beta [1] = 1.0/3.0;
      beta [2] = 1.0/2.0;
      beta [3] = 1.0;

      gamma[0] = 0.0;
      gamma[1] = 0.5;
      gamma[2] = 0.5;
      gamma[3] = 1.0;
      break;
  }

  if (gamma[0] != 0) throw BadValue(FromHere(),"gamma[0] must be zero for consistent time marching");

  options().set("alpha",alpha);
  options().set("beta",beta);
  options().set("gamma",gamma);
}

///////////////////////////////////////////////////////////////////////////////////////

void ExplicitRungeKuttaLowStorage2::link_fields()
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
}

///////////////////////////////////////////////////////////////////////////////////////

void ExplicitRungeKuttaLowStorage2::execute()
{
  configure_option_recursively( "iterator", handle<Component>() );

  link_fields();

  int convergence_failed = false;
  const Uint nb_stages = options().value<Uint>("nb_stages");
  std::vector<Real> alpha = options().value< std::vector<Real> >("alpha");
  std::vector<Real> beta  = options().value< std::vector<Real> >("beta");
  std::vector<Real> gamma = options().value< std::vector<Real> >("gamma");
  Field& U  = *m_solution;
  Field& U0 = *m_solution_backup;
  Field& R  = *m_residual;
  Field& H  = *m_update_coeff;

  if (is_null(m_time))        throw SetupError(FromHere(), "Time was not set");
  Time& time = *m_time;

  U0 = U;

  const Real T0 = time.current_time();
  Real dt = 0;

  for (Uint stage=0; stage<nb_stages; ++stage)
  {
    // Set time and iteration for this stage
    properties().property("iteration") = stage+1;
    time.current_time() = T0 + gamma[stage] * dt;

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

    if (stage == 0)
    {
      solver().handle<SDSolver>()->actions().get_child("compute_update_coefficient")->handle<common::Action>()->execute();
    }
    // now assigned:

    // - H
    // - time.dt()

    const Real one_minus_alpha = 1. - alpha[stage];
    boost_foreach(const Handle<Entities>& elements_handle, U.entities_range())
    {
      Entities& elements = *elements_handle;
      const Connectivity& space_connectivity = U.space(elements).connectivity();

      for (Uint e=0; e<elements.size(); ++e)
      {
        boost_foreach(const Uint state, space_connectivity[e])
        {
          for (Uint var=0; var<U.row_size(); ++var)
          {
            U[state][var] = one_minus_alpha*U0[state][var] + alpha[stage]*U[state][var] + beta[stage]*H[state][0]*R[state][var];
          }
        }
      }
    }

    // U has now been updated

    // Do post-processing per stage after update
    post_update().execute();

    U.synchronize();

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

void ExplicitRungeKuttaLowStorage2::raise_iteration_done()
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
