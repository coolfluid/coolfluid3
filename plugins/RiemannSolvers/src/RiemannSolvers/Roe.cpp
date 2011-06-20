// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "RiemannSolvers/Roe.hpp"
#include "Solver/State.hpp"
#include "Common/OptionT.hpp"

namespace CF {
namespace RiemannSolvers {

using namespace Common;
using namespace Solver;

Common::ComponentBuilder < Roe, RiemannSolver, LibRiemannSolvers > Roe_Builder;

////////////////////////////////////////////////////////////////////////////////

Roe::Roe ( const std::string& name  )
: RiemannSolver(name)
{
  properties()["brief"] = std::string("Approximate Riemann solver Roe");
  properties()["description"] = std::string("Solves the Riemann problem using the Roe scheme");

  property("solution_state").as_option().attach_trigger( boost::bind(&Roe::setup, this) );

  m_properties.add_option(OptionT<std::string>::create("roe_state","Roe State","Component describing the Roe state",std::string("")))
      ->mark_basic()
      ->attach_trigger( boost::bind(&Roe::build_roe_state, this) )
      ->add_tag("roe_state");
}

////////////////////////////////////////////////////////////////////////////////

void Roe::build_roe_state()
{
  if (is_not_null(m_roe_state))
    remove_component(*m_roe_state);
  m_roe_state = create_component( "roe_state" , property("roe_state").value_str() ).as_ptr<State>();
  m_roe_avg_vars = m_roe_state->create_physics();
}

////////////////////////////////////////////////////////////////////////////////

Roe::~Roe()
{
}

////////////////////////////////////////////////////////////////////////////////

void Roe::setup()
{
  State& sol_state = *m_sol_state.lock();

  Uint size = sol_state.size();

  right_eigenvectors.resize(size,size);
  left_eigenvectors.resize(size,size);
  eigenvalues.resize(size);
  abs_jacobian.resize(size,size);
  F_L.resize(size);
  F_R.resize(size);


  m_phys_vars.resize(2);
  m_phys_vars[LEFT]  = sol_state.create_physics();
  m_phys_vars[RIGHT] = sol_state.create_physics();
}

////////////////////////////////////////////////////////////////////////////////

void Roe::solve(const RealVector& left, const RealVector& right, const RealVector& normal ,
                RealVector& interface_flux, Real& left_wave_speed, Real& right_wave_speed)
{
  // convert left, right state to linear physical data
  // For scal advection this is equal

  State& sol_state = *m_sol_state.lock();
  State& roe_state = *m_roe_state;
  Solver::Physics& roe_avg_vars = *m_roe_avg_vars;

  // assumes left and right are in roe states
  sol_state.set_state(left,  *m_phys_vars[LEFT]);
  sol_state.set_state(right, *m_phys_vars[RIGHT]);
  roe_state.linearize(m_phys_vars, roe_avg_vars);

  // right eigenvectors
  sol_state.compute_fluxjacobian_right_eigenvectors(roe_avg_vars,normal,right_eigenvectors);

  // left eigenvectors = inverse (right_eigenvectors)
  sol_state.compute_fluxjacobian_left_eigenvectors(roe_avg_vars,normal,left_eigenvectors);

  // eigenvalues
  sol_state.compute_fluxjacobian_eigenvalues(roe_avg_vars,normal,eigenvalues);

  // calculate absolute jacobian
  abs_jacobian = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;

  // flux = central part + upwind part
  sol_state.compute_flux(*m_phys_vars[LEFT] ,normal,F_L);
  sol_state.compute_flux(*m_phys_vars[RIGHT],normal,F_R);

  interface_flux = 0.5*(F_L + F_R) - 0.5*abs_jacobian*(right-left);
  left_wave_speed  = sol_state.max_eigen_value( roe_avg_vars, normal );
  right_wave_speed = sol_state.max_eigen_value( roe_avg_vars, -normal );
}

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // CF
