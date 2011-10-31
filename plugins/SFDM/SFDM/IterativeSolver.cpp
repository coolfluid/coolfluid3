// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Signal.hpp"
#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/EventHandler.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"
#include "common/ActionDirector.hpp"
#include "common/FindComponents.hpp"

#include "math/VariablesDescriptor.hpp"

#include "solver/CTime.hpp"
#include "solver/CSolver.hpp"

#include "solver/actions/CCriterion.hpp"
#include "solver/actions/CCriterionMaxIterations.hpp"

#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Space.hpp"

#include "SFDM/IterativeSolver.hpp"
#include "SFDM/Tags.hpp"

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

  m_properties.add_property( "iteration", Uint(0) );

  // static components

  m_pre_update = create_static_component_ptr<ActionDirector>("PreUpdate");

  m_post_update = create_static_component_ptr<ActionDirector>("PostUpdate");

  m_options.add_option< OptionT<Uint> >("rk_order", 1u)
      ->description("Order of the Runge-Kutta integration")
      ->pretty_name("RK Integration Order")
      ->attach_trigger( boost::bind( &IterativeSolver::config_rk_order , this ) );

  m_options.add_option( OptionComponent<Field>::create(SFDM::Tags::solution(), &m_solution))
      ->description("Solution to update")
      ->pretty_name("Solution");

      m_options.add_option( OptionComponent<Field>::create("solution_backup", &m_solution_backup))
      ->description("Solution Backup")
      ->pretty_name("Solution Backup");

      m_options.add_option( OptionComponent<Field>::create(SFDM::Tags::update_coeff(), &m_update_coeff))
      ->description("Update coefficient")
      ->pretty_name("Update Coefficient");

      m_options.add_option( OptionComponent<Field>::create(SFDM::Tags::residual(), &m_residual))
      ->description("Residual")
      ->pretty_name("Residual");

  std::vector<Real> dummy(4);
  m_options.add_option< OptionArrayT<Real> >("alpha", dummy)
      ->description("RK coefficients alpha")
      ->pretty_name("alpha")
      ->link_to(&m_alpha);

  m_options.add_option< OptionArrayT<Real> >("beta", dummy)
      ->description("RK coefficients beta")
      ->pretty_name("beta")
      ->link_to(&m_beta);

  m_options.add_option< OptionArrayT<Real> >("gamma", dummy)
      ->description("RK coefficients gamma")
      ->pretty_name("gamma")
      ->link_to(&m_gamma);

  options().add_option( OptionComponent<CTime>::create( SFDM::Tags::time(), &m_time))
      ->description("Time component")
      ->pretty_name("Time");

  config_rk_order();
}

///////////////////////////////////////////////////////////////////////////////////////

void IterativeSolver::config_rk_order()
{
  const Uint nb_stages = option("rk_order").value<Uint>();

  std::vector<Real> alpha(nb_stages);
  std::vector<Real> beta(nb_stages);
  std::vector<Real> gamma(nb_stages);

  // Set defaults Values
  switch (nb_stages)
  {
    case 1: // Simple Forward Euler
      alpha[0] = 0.0;
      beta[0] = 1.0;
      gamma[0] = 0.0;
      break;

    case 2: // R-K 2
      alpha[0] = 0.0;
      alpha[1] = 0.0;

      beta[0] = 0.5;
      beta[1] = 1.0;

      gamma[0] = 0.0;
      gamma[1] = 0.5;
      break;

    case 3:  // 3rd order TVD R-K scheme
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
    default:
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

  configure_option("alpha",alpha);
  configure_option("beta",beta);
  configure_option("gamma",gamma);
}

///////////////////////////////////////////////////////////////////////////////////////

void IterativeSolver::link_fields()
{
  if (m_solution.expired())
    m_solution = solver().field_manager().get_child( SFDM::Tags::solution() ).follow()->as_ptr_checked<Field>();
  if (m_residual.expired())
    m_residual = solver().field_manager().get_child( SFDM::Tags::residual() ).follow()->as_ptr_checked<Field>();
  if (m_update_coeff.expired())
    m_update_coeff = solver().field_manager().get_child( SFDM::Tags::update_coeff() ).follow()->as_ptr_checked<Field>();
  if ( m_solution_backup.expired() )  // backup not created --> create field
  {
    if (Component::Ptr found_solution_backup = solver().field_manager().get_child_ptr( "solution_backup" ))
    {
      m_solution_backup = found_solution_backup->follow()->as_ptr_checked<Field>();
    }
    else if ( Component::Ptr found_solution_backup = m_solution.lock()->field_group().get_child_ptr( "solution_backup" ) )
    {
      solver().field_manager().create_component<Link>("solution_backup").link_to(found_solution_backup);
      m_solution_backup = found_solution_backup->as_ptr<Field>();
    }
    else
    {
      m_solution_backup = m_solution.lock()->field_group().create_field("solution_backup", m_solution.lock()->descriptor().description()).as_ptr<Field>();
      m_solution_backup.lock()->descriptor().prefix_variable_names("backup_");
      solver().field_manager().create_component<Link>("solution_backup").link_to(*m_solution_backup.lock());
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void IterativeSolver::execute()
{

  /// @todo these configurations sould be in constructor but does not work there
  ///       because uri() is undefined on the constructor ( component is still free )
  configure_option_recursively( "iterator", this->uri() );

  link_fields();

  const Uint nb_stages = option("rk_order").value<Uint>();
  std::vector<Real> alpha = option("alpha").value< std::vector<Real> >();
  std::vector<Real> beta  = option("beta").value< std::vector<Real> >();
  std::vector<Real> gamma = option("gamma").value< std::vector<Real> >();
  Field& U  = *m_solution.lock();
  Field& U0 = *m_solution_backup.lock();
  Field& R  = *m_residual.lock();
  Field& H  = *m_update_coeff.lock();

  if (m_time.expired())        throw SetupError(FromHere(), "Time was not set");
  CTime& time = *m_time.lock();

  U0 = U;

  const Real T0 = time.current_time();
  Real dt = 0;
  pre_update().configure_option_recursively("freeze_update_coeff",false);

  for (Uint stage=0; stage<nb_stages; ++stage)
  {
    // Set time and iteration for this stage
    property("iteration") = stage+1;
    time.current_time() = T0 + gamma[stage] * dt;

    // Do actual computations in pre_update
    pre_update().execute();

    // now assigned in pre-update
    // - R
    // - H
    // - time.dt()
    // Runge-Kutta UPDATE
    const Real one_minus_alpha = 1. - alpha[stage];
    boost_foreach(const Entities& elements, U.entities_range())
    {
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
      pre_update().configure_option_recursively("freeze_update_coeff",true);
    }
    else
    {
      time.dt() = dt;
    }
    time.current_time() = T0;
    // raise signal that iteration is done
    //raise_iteration_done();
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void IterativeSolver::raise_iteration_done()
{
  SignalOptions opts;
  const Uint iter = properties().value<Uint>("iteration");
  opts.add_option< OptionT<Uint> >( "iteration", iter );
  SignalFrame frame = opts.create_frame("iteration_done", uri(), URI());

  common::Core::instance().event_handler().raise_event( "iteration_done", frame);
}

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3
