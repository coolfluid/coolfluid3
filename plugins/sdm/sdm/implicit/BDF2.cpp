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

#include "math/Consts.hpp"
#include "math/VariablesDescriptor.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

#include "sdm/System.hpp"
#include "sdm/DomainDiscretization.hpp"
#include "sdm/ComputeCellJacobianPerturb.hpp"
#include "sdm/implicit/BDF2.hpp"
#include "sdm/TimeIntegrationStepComputer.hpp"

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace sdm {
namespace implicit {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < BDF2, System, LibImplicit > BDF2_Builder;

////////////////////////////////////////////////////////////////////////////////

BDF2::BDF2 ( const std::string& name ) :
  System(name)
{
  CFdebug << "Creating BDF2" << CFendl;

  // Real options that are used
  options().add("domain_discretization",m_domain_discretization).link_to(&m_domain_discretization);
  options().add("solution",m_solution).link_to(&m_solution).attach_trigger( boost::bind( &BDF2::create_solution_backups , this) );
  options().add("residual",m_residual).link_to(&m_residual);
  options().add("update_coefficient", m_update_coeff)
      .description("Update coefficient to multiply with residual")
      .pretty_name("Update Coefficient")
      .attach_trigger( boost::bind( &BDF2::create_update_coeff_backups , this) )
      .link_to(&m_update_coeff);

  options().add("time_step_computer",m_time_step_computer).link_to(&m_time_step_computer).mark_basic();

  // Create the component that will compute the cell jacobian
  m_compute_jacobian = create_static_component<ComputeCellJacobianPerturb>("ComputeCellJacobian");
  m_compute_jacobian->mark_basic();

  CFdebug << "Created BDF2" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

void BDF2::configure()
{
  if ( is_null(m_domain_discretization) ) throw SetupError( FromHere(), "Option 'domain_discretization' not configured for "+uri().string());
  if ( is_null(m_solution) )              throw SetupError( FromHere(), "Option 'solution' not configured for "+uri().string());
  if ( is_null(m_residual) )              throw SetupError( FromHere(), "Option 'residual' not configured for "+uri().string());
  if ( is_null(m_update_coeff) )          throw SetupError( FromHere(), "Option 'update_coefficient' not configured for "+uri().string());

  // configure m_compute_jacobian
  m_compute_jacobian->options().set("domain_discretization",m_domain_discretization);
  m_compute_jacobian->options().set("solution",m_solution);
  m_compute_jacobian->options().set("residual",m_residual);

}

////////////////////////////////////////////////////////////////////////////////

void BDF2::create_solution_backups()
{
  if ( is_null(m_solution) )              throw SetupError( FromHere(), "Option 'solution' not configured for "+uri().string());

  if ( is_null(m_solution->dict().get_child("solution_backup")) )
  {
    m_solution_backup = m_solution->dict().create_field("solution_backup",m_solution->descriptor().description()).handle<Field>();
    *m_solution_backup = *m_solution;
  }
  else
  {
    m_solution_backup = m_solution->dict().get_child("solution_backup")->handle<Field>();
  }

  if ( is_null(m_solution->dict().get_child("solution_previous")) )
  {
    m_solution_previous = m_solution->dict().create_field("solution_previous",m_solution->descriptor().description()).handle<Field>();
    *m_solution_previous = *m_solution_backup;
  }
  else
  {
    m_solution_previous = m_solution->dict().get_child("solution_previous")->handle<Field>();
  }
}

////////////////////////////////////////////////////////////////////////////////

void BDF2::create_update_coeff_backups()
{
  if ( is_null(m_update_coeff) )              throw SetupError( FromHere(), "Option 'update_coefficient' not configured for "+uri().string());

  if ( is_null(m_update_coeff->dict().get_child("update_coeff_previous")) )
  {
    m_update_coeff_previous = m_update_coeff->dict().create_field("update_coeff_previous",m_update_coeff->descriptor().description()).handle<Field>();
  }
  else
  {
    m_update_coeff_previous = m_update_coeff->dict().get_child("update_coeff_previous")->handle<Field>();
  }
}

////////////////////////////////////////////////////////////////////////////////

Real BDF2::coeff_c1(const Real& dt_n, const Real& dt_nm1) const
{
  if (dt_nm1 == 0.)   // This is the case of the very first time-level "n=0"
    return 1.;        // this will make it BackwardEuler

  const Real tau = dt_n/dt_nm1;
  return (1. + tau)/(1.+2.*tau);
}

////////////////////////////////////////////////////////////////////////////////

Real BDF2::coeff_c2(const Real& dt_n, const Real& dt_nm1) const
{
  if (dt_nm1 == 0.)   // This is the case of the very first time-level "n=0"
    return 0.;        // this will make it BackwardEuler

  const Real tau = dt_n/dt_nm1;
  return (tau*tau)/( dt_n * (1.+2.*tau) );
}

////////////////////////////////////////////////////////////////////////////////

void BDF2::prepare()
{
  // Try to auto-configure in case this did not happen yet
  configure();

  // It is assumed that the m_solution_backup is still at time "n-1",
  // and m_solution at time "n"

  // update previous solution to be at time "n-1"
  *m_solution_previous = *m_solution_backup;
  *m_update_coeff_previous = *m_update_coeff;

  // update solution backup to be at time "n"
  *m_solution_backup = *m_solution;

  // compute residual at time "n", plus wave-speeds! --> used to compute dt
  m_domain_discretization->options().set("solution",m_solution);
//  m_domain_discretization->options().set("wave_speed",m_wave_speed);
  m_domain_discretization->options().set("residual",m_residual);
  m_domain_discretization->execute();

  // compute the time-accurate time step or non-time-accurate update-coefficients per DOF
  m_time_step_computer->options().set("update_coefficient",m_update_coeff);
  m_time_step_computer->execute();
}

////////////////////////////////////////////////////////////////////////////////

void BDF2::synchronize()
{
  m_solution->synchronize();
}

////////////////////////////////////////////////////////////////////////////////

Uint BDF2::nb_rows() const
{
  if ( is_null(m_solution) )  throw SetupError( FromHere(), "Option 'solution' not configured for "+uri().string());
  cf3_assert_desc("Must first call loop_cells() function", is_not_null(m_space) );
  return m_space->shape_function().nb_nodes() * m_solution->row_size();
}

////////////////////////////////////////////////////////////////////////////////

Uint BDF2::nb_cols() const
{
  if ( is_null(m_solution) )  throw SetupError( FromHere(), "Option 'solution' not configured for "+uri().string());
  cf3_assert_desc("Must first call loop_cells() function", is_not_null(m_space) );
  return m_space->shape_function().nb_nodes() * m_solution->row_size();
}

////////////////////////////////////////////////////////////////////////////////

bool BDF2::loop_cells(const Handle<mesh::Entities const>& cells)
{
  if ( is_null(cells->handle<mesh::Cells>()) ) return false;
  if (m_compute_jacobian->loop_cells(cells->handle<Cells>()) == false)
    return false;

  m_space = m_solution->space(cells);
  cf3_assert(m_space);

  m_nb_vars = m_solution->row_size();
  m_nb_sol_pts = m_space->shape_function().nb_nodes();

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void BDF2::compute_lhs(const Uint elem, RealMatrix& lhs)
{
  /// @f[ lhs = - c_1\ \frac{\partial R}{\partial Q_n}(Q^n) + \frac{I}{\Delta t} @f]
  //                    n
  //                dR(Q )     I
  // lhs  =   - c1  ------  +  --
  //                  dQ       dt
  //
  cf3_assert(lhs.rows() == nb_rows());
  cf3_assert(lhs.cols() == nb_cols());

  // store dR/dQ in lhs
  m_compute_jacobian->compute_jacobian(elem,lhs);

  // add I/dt
  const Field& H_n   = *m_update_coeff;
  const Field& H_nm1 = *m_update_coeff_previous;
  for (Uint s=0; s<m_nb_sol_pts; ++s)
  {
    const Uint p = m_space->connectivity()[elem][s];

    const Real& dt_n   = H_n[p][0];
    const Real& dt_nm1 = H_nm1[p][0];
    const Real  c1 = coeff_c1(dt_n,dt_nm1);
    const Real  inv_dt = 1./dt_n;

    for (Uint v=0; v<m_nb_vars; ++v)
    {

      // multiply row with  (- c1)
      for (Uint ds=0; ds<m_nb_sol_pts; ++ds)
      {
        for (Uint dv=0; dv<m_nb_vars; ++dv)
        {
          lhs(s*m_nb_vars+v,ds*m_nb_vars+dv) *= - c1;
        }
      }

      // add 1/dt to diagonal of this row
      lhs(s*m_nb_vars+v,s*m_nb_vars+v) += inv_dt;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void BDF2::compute_rhs(const Uint elem, RealVector &rhs)
{
  /// @f[ rhs = R(Q) - \frac{Q-Q^n}{\Delta t}  @f]
  //                                                    n + 1,k    n
  //                n + 1,k            n    n - 1      Q        - Q
  // rhs  =  c1  R(Q       )   + c2  (Q  - Q     )  -  -------------
  //                                                      Delta t
  const Field& Q_np1 = *m_solution;
  const Field& Q_n   = *m_solution_backup;
  const Field& Q_nm1 = *m_solution_previous;
  const Field& H_n   = *m_update_coeff;
  const Field& H_nm1 = *m_update_coeff_previous;
  const Field& R_np1 = *m_residual;

  // Recompute R for this element
  m_domain_discretization->compute_element(elem);

  for (Uint s=0; s<m_nb_sol_pts; ++s)
  {
    const Uint p = m_space->connectivity()[elem][s];

    const Real& dt_n   = H_n[p][0];
    const Real& dt_nm1 = H_nm1[p][0];
    const Real  c1 = coeff_c1(dt_n,dt_nm1);
    const Real  c2 = coeff_c2(dt_n,dt_nm1);

    for (Uint v=0; v<m_nb_vars; ++v)
    {
      rhs[s*m_nb_vars+v] = c1 * R_np1[p][v] + c2 * (Q_n[p][v] - Q_nm1[p][v]) - (Q_np1[p][v] - Q_n[p][v])/dt_n;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

Real BDF2::update(const Uint elem, const RealVector& dQ)
{
  cf3_assert( is_not_null(m_space) );
  Field& Q_np1 = *m_solution;
  Real convergence (0.);

  // Update solution register, and compute convergence
  for (Uint s=0; s<m_nb_sol_pts; ++s)
  {
    const Uint p = m_space->connectivity()[elem][s];
    for (Uint v=0; v<m_nb_vars; ++v)
    {
      Q_np1[p][v] += dQ[s*m_nb_vars+v];
      convergence = std::max( convergence, std::abs(dQ[s*m_nb_vars+v]/(math::Consts::eps()+Q_np1[p][v])) );
    }
  }
  return convergence;
}

////////////////////////////////////////////////////////////////////////////////

} // implicit
} // sdm
} // cf3
