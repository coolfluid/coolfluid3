// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionT.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CGroupActions.hpp"
#include "Common/CGroup.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/MeshMetadata.hpp"
#include "Solver/FlowSolver.hpp"
#include "Solver/CTime.hpp"
#include "Solver/CModel.hpp"
#include "Solver/Actions/CAdvanceTime.hpp"
#include "RungeKutta/RK.hpp"
#include "RungeKutta/UpdateSolution.hpp"

namespace CF {
namespace RungeKutta {

using namespace Common;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;

ComponentBuilder<RK, CAction, LibRungeKutta> RK_builder;

////////////////////////////////////////////////////////////////////////////////

RK::RK ( const std::string& name  )
  : Solver::Action(name),
    m_stages(4u)
{
  properties()["brief"] = std::string("Runge Kutta differential equation solver");
  properties()["description"] = std::string("Solves the differential equation using Runge Kutta method");

  properties().add_option(OptionT<Uint>::create("stages","Stages","Number of stages used in the multistage method",m_stages))
      ->link_to(&m_stages)
      ->attach_trigger( boost::bind( &RK::config_stages, this) )
      ->mark_basic();
  config_stages();

  properties().add_option(OptionComponent<CField>::create(FlowSolver::Tags::solution(),"Solution","Solution",&m_solution));
  properties().add_option(OptionComponent<CField>::create(FlowSolver::Tags::residual(),"Residual","Residual",&m_residual));
  properties().add_option(OptionComponent<CField>::create(FlowSolver::Tags::update_coeff(),"Update Coefficient","Update Coefficient",&m_update_coeff));

  m_for_each_stage = create_static_component_ptr<CGroup>("1_for_each_stage");
  m_for_each_stage->mark_basic();
  m_pre_update  = m_for_each_stage->create_static_component_ptr<CGroupActions>("1_pre_update_actions");
  m_pre_update->mark_basic();
  m_update      = m_for_each_stage->create_static_component_ptr<UpdateSolution>("2_update");
  m_update->mark_basic();
  m_post_update = m_for_each_stage->create_static_component_ptr<CGroupActions>("3_post_update_actions");
  m_post_update->mark_basic();
  m_advance_time = create_static_component_ptr<CAdvanceTime>("2_advance_time");
  m_advance_time->mark_basic();

}

////////////////////////////////////////////////////////////////////////////////

RK::~RK()
{
}

void RK::config_stages()
{
  m_alpha.resize(m_stages);
  m_beta .resize(m_stages);
  m_gamma.resize(m_stages);

  // Set defaults Values
  switch (m_stages)
  {
    case 1: // Simple Forward Euler
      m_alpha[0] = 0.0;
      m_beta[0] = 1.0;
      m_gamma[0] = 0.0;
      break;

    case 2: // R-K 2
      m_alpha[0] = 0.0;
      m_alpha[1] = 0.0;

      m_beta[0] = 0.5;
      m_beta[1] = 1.0;

      m_gamma[0] = 0.0;
      m_gamma[1] = 0.5;
      break;

    case 3:  // 3rd order TVD R-K scheme
      m_alpha[0] = 0.0;
      m_alpha[1] = 1.0/4.0;
      m_alpha[2] = 2.0/3.0;

      m_beta[0] = 1.0;
      m_beta[1] = 1.0/4.0;
      m_beta[2] = 2.0/3.0;

      m_gamma[0] = 0.0;
      m_gamma[1] = 0.5;
      m_gamma[2] = 1.0;
      break;

    case 4:    // R-K 4
    default:
      m_alpha.resize(m_stages);
      m_beta .resize(m_stages);
      m_gamma.resize(m_stages);

      m_alpha[0] = 0.0;
      m_alpha[1] = 0.0;
      m_alpha[2] = 0.0;
      m_alpha[3] = 0.0;

      m_beta [0] = 1.0/4.0;
      m_beta [1] = 1.0/3.0;
      m_beta [2] = 1.0/2.0;
      m_beta [3] = 1.0;

      m_gamma[0] = 0.0;
      m_gamma[1] = 0.5;
      m_gamma[2] = 0.5;
      m_gamma[3] = 1.0;
      break;
  }

  if (m_gamma[0] != 0) throw BadValue(FromHere(),"gamma[0] must be zero for consistent time marching");
  // This is because of the line  --> time().time() = T0 + m_gamma[k]*time().dt();
  // in the stages loop.
}

////////////////////////////////////////////////////////////////////////////////

void RK::execute()
{
  if (m_solution.expired()) throw SetupError (FromHere(), "solution was not set");

  if ( m_solution_backup.expired() )  // backup not created --> create field
    m_solution_backup = mesh().create_field("solution_backup",*m_solution.lock()).as_ptr<CField>();

  const CTable<Real>& U  = m_solution.lock()->data();
  CTable<Real>&       U0 = m_solution_backup.lock()->data();

  /// @todo put this in triggers of own config options
  m_update->configure_property("solution",m_solution.lock()->uri());
  m_update->configure_property("solution_backup",m_solution_backup.lock()->uri());
  m_update->configure_property("residual",m_residual.lock()->uri());
  m_update->configure_property("update_coeff",m_update_coeff.lock()->uri());

  /// 1) backup solution and time
  U0 = U;
  const Real T0 = time().time();

  /// For every stage of the Runge Kutta scheme
  m_pre_update->configure_option_recursively("freeze_update_coeff",false);
  for (Uint k=0; k<m_stages; ++k)
  {
    /// - Set the time for this stage (notice that at first stage time is not modified since m_gamma[0] = 0)
    time().time() = T0 + m_gamma[k]*time().dt();

    /// - Pre update actions, must compute residual, update_coefficient (and thus time().dt())
    m_pre_update->execute();

    /// - Freeze update_coeff for following stages
    if (k==0) m_pre_update->configure_option_recursively("freeze_update_coeff",true);

    /// - Update solution
    ///   @f[ U^{k+1} = (1-\alpha_k)\ U^0 + \alpha_k \ U^k + \beta_k H \ R(U^k) @f]
    ///   with @f$ H @f$ the delta of the ODE
    m_update->set_coefficients(m_alpha[k],m_beta[k]);
    m_update->execute();

    /// - Post update actions, filters, checks, ...
    m_post_update->execute();
  }
  /// Set time back to pre-stages time, so that the action Solver::CAdvanceTime will update the time
  /// @note that time().dt() has been modified
  time().time() = T0;
  m_advance_time->execute();
}

////////////////////////////////////////////////////////////////////////////////

} // RungeKutta
} // CF
