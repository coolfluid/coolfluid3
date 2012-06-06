// Copyright (C) 2010-2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/EventHandler.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/FindComponents.hpp"
#include "common/Group.hpp"

#include "math/VariablesDescriptor.hpp"

#include "solver/Time.hpp"
#include "solver/Solver.hpp"

#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"

#include "sdm/SDSolver.hpp"

#include "sdm/explicit_rungekutta/ExplicitRungeKutta.hpp"
#include "sdm/explicit_rungekutta/Types.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::mesh;

namespace cf3 {
namespace sdm {
namespace explicit_rungekutta {

////////////////////////////////////////////////////////////////////////////////

ExplicitRungeKuttaBase::ExplicitRungeKuttaBase ( const std::string& name ) :
  IterativeSolver(name)
{
}

///////////////////////////////////////////////////////////////////////////////////////

void ExplicitRungeKuttaBase::link_fields()
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

  Uint nb_stages = m_butcher->nb_stages();
  if (m_residuals.size() != nb_stages)
    m_residuals.resize(nb_stages);
  for (Uint i=0; i<nb_stages; ++i)
  {
    if ( is_null(m_residuals[i]) )  // residual not created --> create field
    {
      if (Handle< Component > found_residual = solver().field_manager().get_child( "erk_eval_"+to_str(i) ))
      {
        m_residuals[i] = Handle<Field>( follow_link(found_residual) );
      }
      else if ( Handle< Component > found_residual = m_solution->dict().get_child( "erk_eval_"+to_str(i) ) )
      {
        solver().field_manager().create_component<Link>("erk_eval_"+to_str(i))->link_to(*found_residual);
        m_residuals[i] = found_residual->handle<Field>();
      }
      else
      {
        m_residuals[i] = m_solution->dict().create_field("erk_eval_"+to_str(i), m_solution->descriptor().description()).handle<Field>();
        m_residuals[i]->descriptor().prefix_variable_names("erk_eval_"+to_str(i)+"_");
        solver().field_manager().create_component<Link>("erk_eval_"+to_str(i))->link_to(*m_residuals[i]);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void ExplicitRungeKuttaBase::execute()
{
  cf3_assert (is_not_null(m_butcher));
  configure_option_recursively( "iterator", handle<Component>() );
  
  const ButcherTableau& butcher = *m_butcher;
  butcher.check_throw();
  const Uint nb_stages = butcher.nb_stages();
  const Uint last_stage = nb_stages-1;


  link_fields();


  int convergence_failed = false;
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
    time.current_time() = T0 + butcher.c(stage) * dt;

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
    Field& R_stage = (*m_residuals[stage]);
    R_stage = R;


    if (stage == 0)
    {
      solver().handle<SDSolver>()->actions().get_child("compute_update_coefficient")->handle<common::Action>()->execute();
    }
    // now assigned:

    // - H
    // - time.dt()

    if (stage != last_stage)  // update solution for next stage
    {
      Uint next_stage = stage+1;
      /// U(s+1) = U(n) + h * sum( asj * Rj )
      /// R = sum( asj * Rj )
      U = U0;
      R = 0;
      Real r;
      for (Uint j=0; j<next_stage; ++j)
      {
        Real a = butcher.a(next_stage,j);
        if (a != 0)
        {
          const Field& R_j = (*m_residuals[j]);  // R from previous stages
          for (Uint pt=0; pt<U.size(); ++pt)
          {
            for (Uint v=0; v<U.row_size(); ++v)
            {
              r =  a * R_j[pt][v];
              R[pt][v] += r;
              U[pt][v] += H[pt][0] * r;
            }
          }
        }
      }
    }
    else // weighted average of all stages forms final solution
    {
      /// U(n+1) = U(n) + h * sum( bj * Rj )
      /// R = sum( bj * Rj )
      U = U0;
      R = 0.;
      Real r;
      for (Uint j=0; j<nb_stages; ++j)
      {
        if (butcher.b(j)!=0)
        {
          const Field& R_j = (*m_residuals[j]);  // R from previous stages
          for (Uint pt=0; pt<U.size(); ++pt)
          {
            for (Uint v=0; v<U.row_size(); ++v)
            {
              r = butcher.b(j) * R_j[pt][v];
              R[pt][v] += r;
              U[pt][v] += H[pt][0] * r;
            }
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

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ExplicitRungeKutta, ExplicitRungeKuttaBase, LibExplicitRungeKutta > ExplicitRungeKutta_Builder;

////////////////////////////////////////////////////////////////////////////////

ExplicitRungeKutta::ExplicitRungeKutta ( const std::string& name ) : ExplicitRungeKuttaBase(name)
{
  options().add("order", 4)
      .description("Order of the Runge-Kutta integration (default = RK44)\n"
                   "NOTE: This overrides any existing Butcher tableau")
      .pretty_name("RK order")
      .attach_trigger( boost::bind( &ExplicitRungeKutta::config_butcher_tableau, this ) )
      .mark_basic();

  m_butcher = create_component<ButcherTableau>("butcher_tableau");
  m_butcher->set( butcher_tableau::ClassicRK44() );
}

////////////////////////////////////////////////////////////////////////////////

// Some default coefficients that are configured with option "order"
void ExplicitRungeKutta::config_butcher_tableau()
{
  using namespace explicit_rungekutta;
  const Uint order = options().value<Uint>("order");
  switch (order)
  {
    case 1: // set to forward Euler
      m_butcher->set( butcher_tableau::ForwardEuler() );
      break;
    case 2: // 2-stage 2nd-order method
      m_butcher->set( butcher_tableau::Heun2() );
      break;
    case 3: // Classic RK33
      m_butcher->set( butcher_tableau::ClassicRK33() );
      break;
    case 4: // Classic RK44
      m_butcher->set( butcher_tableau::ClassicRK44() );
      break;
    case 5: // RKF65
      m_butcher->set( butcher_tableau::RKF65() );
      break;
    default:
      CFwarn << "Cannot configure order " << order << ". Using ClassicRK44 instead." << CFendl;
      m_butcher->set( butcher_tableau::ClassicRK44() );
      break;
  }
  CFinfo << "Used Butcher tableau:\n" << m_butcher->str() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

} // explicit_rungekutta
} // sdm
} // cf3
