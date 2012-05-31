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

#include "sdm/ExplicitRungeKutta.hpp"
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

common::ComponentBuilder < ExplicitRungeKutta, common::Action, LibSDM > ExplicitRungeKutta_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ExplicitRungeKutta::ExplicitRungeKutta ( const std::string& name ) :
  IterativeSolver(name)
{
  mark_basic();

  options().add("order", 4u)
      .description("Order of the Runge-Kutta integration (default = RK4)\n"
                   "NOTE: Configure coefficients a,b,c after configuring this to override defaults")
      .pretty_name("RK order")
      .attach_trigger( boost::bind( &ExplicitRungeKutta::config_coefficients, this ) )
      .mark_basic();

  options().add("nb_stages", 0u)
      .description("Number of stages of the Runge-Kutta integration")
      .pretty_name("RK stages");

  options().add("a", std::vector<Real>())
      .description("RK coefficients a from Butcher tableau")
      .link_to(&m_a);

  options().add("b", std::vector<Real>())
      .description("RK coefficients b from Butcher tableau")
      .link_to(&m_b);

  options().add("c", std::vector<Real>())
      .description("RK coefficients c from Butcher tableau")
      .link_to(&m_c);

  // Configure default coefficients
  config_coefficients();
}

////////////////////////////////////////////////////////////////////////////////

// Some default coefficients that are configured with option "order"
void ExplicitRungeKutta::config_coefficients()
{
  const Uint nb_stages = options().value<Uint>("order");
  options().set("nb_stages",nb_stages);

  switch (nb_stages)
  {
    case 1: // set to Forward Euler
    {
      // 0 |
      // ------
      //   | 1
      Real a[] = { 0. };
      Real b[] = { 1. };
      Real c[] = { 0. };
      options().set("a",std::vector<Real>(a,a+nb_stages*nb_stages));
      options().set("b",std::vector<Real>(b,b+nb_stages));
      options().set("c",std::vector<Real>(c,c+nb_stages));
      break;
    }
    case 2: // Heun method
    {
      // 0   |
      // 2/3 | 2/3
      // --------------
      //     | 1/3  3/4
      Real a[] = { 0.,    0.,
                   2./3., 0.   };
      Real b[] = { 1./3., 3./4.};
      Real c[] = { 0.,    2./3.};
      options().set("a",std::vector<Real>(a,a+nb_stages*nb_stages));
      options().set("b",std::vector<Real>(b,b+nb_stages));
      options().set("c",std::vector<Real>(c,c+nb_stages));
      break;
    }
    case 3: // RK3
    {
      // 0   |
      // 1/2 | 1/2
      // 1   | -1    2
      // -------------------
      //     | 1/6  2/3  1/6
      Real a[] = { 0.,    0.,    0.,
                   1./2., 0.,    0.,
                   -1.,   2.,    0.   };
      Real b[] = { 1./6., 2./3., 1./6.};
      Real c[] = { 0.,    1./2., 1.   };
      options().set("a",std::vector<Real>(a,a+nb_stages*nb_stages));
      options().set("b",std::vector<Real>(b,b+nb_stages));
      options().set("c",std::vector<Real>(c,c+nb_stages));
      break;
    }
    case 4: // Classic RK4
    default:
    {
      // 0   |
      // 1/2 | 1/2
      // 1/2 | 0    1/2
      // 1   | 0    0    1
      // ------------------------
      //     | 1/6  1/3  1/3  1/6
      //
      Real a[] = { 0.,    0.,    0.,    0.,
                   1./2., 0.,    0.,    0.,
                   0.,    1./2., 0.,    0.,
                   0.,    0.,    1.,    0.   };
      Real b[] = { 1./6., 1./3., 1./3., 1./6.};
      Real c[] = { 0.,    1./2., 1./2., 1.   };
      options().set("a",std::vector<Real>(a,a+nb_stages*nb_stages));
      options().set("b",std::vector<Real>(b,b+nb_stages));
      options().set("c",std::vector<Real>(c,c+nb_stages));
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void ExplicitRungeKutta::link_fields()
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

  Uint nb_stages = options().value<Uint>("nb_stages");
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

void ExplicitRungeKutta::execute()
{
  configure_option_recursively( "iterator", handle<Component>() );
  
  link_fields();

  int convergence_failed = false;
  const Uint nb_stages = options().value<Uint>("nb_stages");
  const Uint last_stage = nb_stages-1;
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
    time.current_time() = T0 + m_c[stage] * dt;

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

//    Handle<common::Action> compute_norm = solver().handle<SDSolver>()->actions()
//                             .get_child(Tags::L2norm())->handle<common::Action>();
//    compute_norm->options().set("table",R_stage.uri());
//    compute_norm->execute();
//    std::vector<Real> norm = compute_norm->properties().value< std::vector<Real> >("norms");
//    std::cout << "norm = " << to_str(norm) << std::endl;


    if (stage == 0)
    {
      solver().handle<SDSolver>()->actions().get_child("compute_update_coefficient")->handle<common::Action>()->execute();
    }
    // now assigned:

    // - H
    // - time.dt()

    if (stage != last_stage)  // update solution for next stage
    {
      /// U(s+1) = U(n) + h * sum( asj * Rj )
      /// R = sum( asj * Rj )
      U = U0;
      R = 0;
      Real r;
      for (Uint j=0; j<=stage; ++j)
      {
        if (m_a[(stage+1)*nb_stages + j] != 0)
        {
          const Field& R_j = (*m_residuals[j]);  // R from previous stages
          for (Uint pt=0; pt<U.size(); ++pt)
          {
            for (Uint v=0; v<U.row_size(); ++v)
            {
              r =  m_a[(stage+1)*nb_stages + j] * R_j[pt][v];
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
        if (m_b[j]!=0)
        {
          const Field& R_j = (*m_residuals[j]);  // R from previous stages
          for (Uint pt=0; pt<U.size(); ++pt)
          {
            for (Uint v=0; v<U.row_size(); ++v)
            {
              r = m_b[j] * R_j[pt][v];
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

///////////////////////////////////////////////////////////////////////////////////////

void ExplicitRungeKutta::raise_iteration_done()
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
