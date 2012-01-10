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

#include "solver/CTime.hpp"
#include "solver/CSolver.hpp"

#include "solver/actions/CCriterion.hpp"
#include "solver/actions/CCriterionMaxIterations.hpp"
#include "solver/actions/CComputeLNorm.hpp"

#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Space.hpp"

#include "SFDM/IterativeSolver.hpp"
#include "SFDM/Tags.hpp"
#include "SFDM/SFDSolver.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::mesh;

namespace cf3 {
namespace SFDM {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < IterativeSolver, common::Action, LibSFDM > IterativeSolver_Builder;

///////////////////////////////////////////////////////////////////////////////////////

IterativeSolver::IterativeSolver ( const std::string& name ) :
  solver::Action(name)
{
  mark_basic();

  // properties

  properties().add_property( "iteration", Uint(0) );

  // static components

  m_pre_update = create_static_component<common::ActionDirector>("PreUpdate");

  m_post_update = create_static_component<common::ActionDirector>("PostUpdate");

  options().add_option("order", 1u)
      .description("Order of the Runge-Kutta integration")
      .pretty_name("RK order");

  options().add_option("nb_stages", 1u)
      .description("Number of stages of the Runge-Kutta integration")
      .pretty_name("RK stages")
      .attach_trigger( boost::bind( &IterativeSolver::config_nb_stages , this ) );

  options().add_option(SFDM::Tags::solution(), m_solution)
      .description("Solution to update")
      .pretty_name("Solution")
      .link_to(&m_solution);

      options().add_option("solution_backup", m_solution_backup)
      .description("Solution Backup")
      .pretty_name("Solution Backup")
      .link_to(&m_solution_backup);

      options().add_option(SFDM::Tags::update_coeff(), m_update_coeff)
      .description("Update coefficient")
      .pretty_name("Update Coefficient")
      .link_to(&m_update_coeff);

      options().add_option(SFDM::Tags::residual(), m_residual)
      .description("Residual")
      .pretty_name("Residual")
      .link_to(&m_residual);

  std::vector<Real> dummy(4);
  options().add_option("alpha", dummy)
      .description("RK coefficients alpha")
      .pretty_name("alpha")
      .link_to(&m_alpha);

  options().add_option("beta", dummy)
      .description("RK coefficients beta")
      .pretty_name("beta")
      .link_to(&m_beta);

  options().add_option("gamma", dummy)
      .description("RK coefficients gamma")
      .pretty_name("gamma")
      .link_to(&m_gamma);

  options().add_option(SFDM::Tags::time(), m_time)
      .description("Time component")
      .pretty_name("Time")
      .link_to(&m_time);

  config_nb_stages();


  CComputeLNorm& cnorm = *create_static_component<CComputeLNorm>( "ComputeNorm" );
  cnorm.options().configure_option("order",2u);
  cnorm.options().configure_option("scale",true);

}

///////////////////////////////////////////////////////////////////////////////////////

void IterativeSolver::config_nb_stages()
{
  const Uint nb_stages = options().option("nb_stages").value<Uint>();

  std::vector<Real> alpha(nb_stages,0);
  std::vector<Real> beta(nb_stages,0);
  std::vector<Real> gamma(nb_stages,0);

  // Set defaults Values
  switch (nb_stages)
  {
    case 1: // Simple Forward Euler
      options().configure_option("order",1u);
      alpha[0] = 0.0;
      beta[0] = 1.0;
      gamma[0] = 0.0;
      break;

    case 2: // R-K 2
      options().configure_option("order",2u);

      alpha[0] = 0.0;
      alpha[1] = 0.0;

      beta[0] = 0.5;
      beta[1] = 1.0;

      gamma[0] = 0.0;
      gamma[1] = 0.5;
      break;

    case 3:  // 3rd order TVD R-K scheme
      options().configure_option("order",3u);

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
      options().configure_option("order",4u);

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

  options().configure_option("alpha",alpha);
  options().configure_option("beta",beta);
  options().configure_option("gamma",gamma);
}

///////////////////////////////////////////////////////////////////////////////////////

void IterativeSolver::link_fields()
{
  if (is_null(m_solution))
    m_solution = Handle<Field>( follow_link( solver().field_manager().get_child( SFDM::Tags::solution() ) ) );
  if (is_null(m_residual))
    m_residual = Handle<Field>( follow_link( solver().field_manager().get_child( SFDM::Tags::residual() ) ) );
  if (is_null(m_update_coeff))
    m_update_coeff = Handle<Field>( follow_link( solver().field_manager().get_child( SFDM::Tags::update_coeff() ) ) );
  if ( is_null(m_solution_backup) )  // backup not created --> create field
  {
    if (Handle< Component > found_solution_backup = solver().field_manager().get_child( "solution_backup" ))
    {
      m_solution_backup = Handle<Field>( follow_link(found_solution_backup) );
    }
    else if ( Handle< Component > found_solution_backup = m_solution->field_group().get_child( "solution_backup" ) )
    {
      solver().field_manager().create_component<Link>("solution_backup")->link_to(*found_solution_backup);
      m_solution_backup = found_solution_backup->handle<Field>();
    }
    else
    {
      m_solution_backup = m_solution->field_group().create_field("solution_backup", m_solution->descriptor().description()).handle<Field>();
      m_solution_backup->descriptor().prefix_variable_names("backup_");
      solver().field_manager().create_component<Link>("solution_backup")->link_to(*m_solution_backup);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void IterativeSolver::execute()
{
  configure_option_recursively( "iterator", handle<Component>() );
  
  link_fields();

  const Uint nb_stages = options().option("nb_stages").value<Uint>();
  std::vector<Real> alpha = options().option("alpha").value< std::vector<Real> >();
  std::vector<Real> beta  = options().option("beta").value< std::vector<Real> >();
  std::vector<Real> gamma = options().option("gamma").value< std::vector<Real> >();
  Field& U  = *m_solution;
  Field& U0 = *m_solution_backup;
  Field& R  = *m_residual;
  Field& H  = *m_update_coeff;

  if (is_null(m_time))        throw SetupError(FromHere(), "Time was not set");
  CTime& time = *m_time;

  U0 = U;

  const Real T0 = time.current_time();
  Real dt = 0;

  for (Uint stage=0; stage<nb_stages; ++stage)
  {
    // Set time and iteration for this stage
    properties().property("iteration") = stage+1;
    time.current_time() = T0 + gamma[stage] * dt;

    // Do actual computations in pre_update
    pre_update().execute();

    // now assigned in pre-update
    // - R

    if (stage == 0)
    {
      solver().handle<SFDSolver>()->actions().get_child("compute_update_coefficient")->handle<common::Action>()->execute();
    }
    // now assigned:

    // - H
    // - time.dt()

    // Runge-Kutta UPDATE
    const Real one_minus_alpha = 1. - alpha[stage];
    boost_foreach(const Handle<Entities>& elements_handle, U.entities_range())
    {
      Entities& elements = *elements_handle;
      Space& solution_space = U.space(elements);
      for (Uint e=0; e<elements.size(); ++e)
      {
        boost_foreach(const Uint state, solution_space.indexes_for_element(e))
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

void IterativeSolver::raise_iteration_done()
{
  SignalOptions opts;
  const Uint iter = properties().value<Uint>("iteration");
  opts.add_option( "iteration", iter );
  SignalFrame frame = opts.create_frame("iteration_done", uri(), URI());

  common::Core::instance().event_handler().raise_event( "iteration_done", frame);
}

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3
