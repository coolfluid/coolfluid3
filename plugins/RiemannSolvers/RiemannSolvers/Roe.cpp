// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "RiemannSolvers/Roe.hpp"
#include "Common/OptionComponent.hpp"

namespace CF {
namespace RiemannSolvers {

using namespace Common;

Common::ComponentBuilder < Roe, RiemannSolver, LibRiemannSolvers > Roe_Builder;

////////////////////////////////////////////////////////////////////////////////

Roe::Roe ( const std::string& name ) : RiemannSolver(name)
{
  options().add_option( OptionComponent<Physics::Variables>::create("roe_vars",&m_roe_vars) )
      ->description("The component describing the Roe variables")
      ->pretty_name("Roe Variables");

  option("phys_model").attach_trigger( boost::bind( &Roe::trigger_phys_model, this) );
}


////////////////////////////////////////////////////////////////////////////////

Roe::~Roe()
{
}

////////////////////////////////////////////////////////////////////////////////

void Roe::trigger_phys_model()
{
  coord.resize(model().ndim());
  grads.resize(model().neqs(),model().ndim());
  p_left  = model().create_properties();
  p_right = model().create_properties();
  p_avg   = model().create_properties();

  roe_left.resize(model().neqs());
  roe_right.resize(model().neqs());
  roe_avg.resize(model().neqs());

  f_left.resize(model().neqs(),model().ndim());
  f_right.resize(model().neqs(),model().ndim());

  eigenvalues.resize(model().neqs());
  right_eigenvectors.resize(model().neqs(),model().neqs());
  left_eigenvectors.resize(model().neqs(),model().neqs());
  abs_jacobian.resize(model().neqs(),model().neqs());
}

////////////////////////////////////////////////////////////////////////////////

void Roe::compute_interface_flux(const RealVector& left, const RealVector& right, const RealVector& normal,
                                 RealVector& flux)
{
  // Compute left and right properties
  solution_vars().compute_properties(coord,left,grads,*p_left);
  solution_vars().compute_properties(coord,right,grads,*p_right);

  // Compute the Roe averaged properties
  // Roe-average = standard average of the Roe-parameter vectors
  roe_vars().compute_variables(*p_left,  roe_left );
  roe_vars().compute_variables(*p_right, roe_right);
  roe_avg = 0.5*(roe_left+roe_right);                // Roe-average is result
  roe_vars().compute_properties(coord, roe_avg, grads, *p_avg);

  // Compute absolute jacobian using Roe averaged properties
  solution_vars().flux_jacobian_eigen_structure(*p_avg,normal,right_eigenvectors,left_eigenvectors,eigenvalues);
  abs_jacobian = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;

  // Compute left and right fluxes
  solution_vars().flux(*p_left , f_left);
  solution_vars().flux(*p_right, f_right);

  // Compute flux at interface composed of central part and upwind part
  flux = 0.5*(f_left*normal+f_right*normal) - 0.5 * abs_jacobian*(right-left);
}

////////////////////////////////////////////////////////////////////////////////

void Roe::compute_interface_flux_and_wavespeeds(const RealVector& left, const RealVector& right, const RealVector& normal,
                                                RealVector& flux, RealVector& wave_speeds)
{
  compute_interface_flux(left,right,normal,flux);
  wave_speeds = eigenvalues;
}

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // CF
