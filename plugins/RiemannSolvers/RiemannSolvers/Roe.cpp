// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionComponent.hpp"

#include "RiemannSolvers/Roe.hpp"

namespace cf3 {
namespace RiemannSolvers {

using namespace common;
using namespace physics;

common::ComponentBuilder < Roe, RiemannSolver, LibRiemannSolvers > Roe_Builder;

////////////////////////////////////////////////////////////////////////////////

Roe::Roe ( const std::string& name ) : RiemannSolver(name)
{
  options().add_option( OptionComponent<physics::Variables>::create("roe_vars",&m_roe_vars) )
      ->description("The component describing the Roe variables")
      ->pretty_name("Roe Variables");

  option("physical_model").attach_trigger( boost::bind( &Roe::trigger_physical_model, this) );
}

////////////////////////////////////////////////////////////////////////////////

Roe::~Roe()
{
}

////////////////////////////////////////////////////////////////////////////////

void Roe::trigger_physical_model()
{
  coord.resize(physical_model().ndim());
  grads.resize(physical_model().neqs(),physical_model().ndim());
  p_left  = physical_model().create_properties();
  p_right = physical_model().create_properties();
  p_avg   = physical_model().create_properties();

  roe_left.resize(physical_model().neqs());
  roe_right.resize(physical_model().neqs());
  roe_avg.resize(physical_model().neqs());

  f_left.resize(physical_model().neqs(),physical_model().ndim());
  f_right.resize(physical_model().neqs(),physical_model().ndim());

  eigenvalues.resize(physical_model().neqs());
  right_eigenvectors.resize(physical_model().neqs(),physical_model().neqs());
  left_eigenvectors.resize(physical_model().neqs(),physical_model().neqs());
  abs_jacobian.resize(physical_model().neqs(),physical_model().neqs());

  central_flux.resize(physical_model().neqs());
  upwind_flux.resize(physical_model().neqs());

  // Try to configure solution_vars automatically
  if (m_solution_vars.expired())
  {
    if (Component::Ptr found_solution_vars = find_component_ptr_recursively_with_name(physical_model(),"solution_vars"))
    {
      configure_option("solution_vars",found_solution_vars->uri());
    }
    else
    {
      CFwarn << "Roe RiemannSolver " << uri().string() << " could not auto-config \"solution_vars\".\n"
             << "Reason: component with name \"solution_vars\" not found in ["<<physical_model().uri().string() << "]\n"
             << "Configure manually" << CFendl;
    }
  }

  // Try to configure roe_vars automatically
  if (m_roe_vars.expired())
  {
    if (Component::Ptr found_roe_vars = find_component_ptr_recursively_with_name(physical_model(),"roe_vars"))
    {
      configure_option("roe_vars",found_roe_vars->uri());
    }
    else if (m_solution_vars.expired() == false)
    {
      configure_option("roe_vars",solution_vars().uri());
      CFwarn << "Roe RiemannSolver " << uri().string() << " auto-configured \"roe_vars\" to \"solution_vars\".\n"
             << "Reason: component with name \"roe_vars\" not found in ["<<physical_model().uri().string() << "].\n"
             << "Configure manually for different \"roe_vars\"" << CFendl;
    }
    else
    {
      CFwarn << "Roe RiemannSolver " << uri().string() << " could not auto-config \"roe_vars\".\n"
             << "Reason: component with name \"roe_vars\" not found in ["<<physical_model().uri().string() << "]\n"
             << "Configure manually" << CFendl;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void Roe::compute_interface_flux(const RealVector& left, const RealVector& right, const RealVector& normal,
                                 RealVector& flux)
{
  physics::Variables& sol_vars = *m_solution_vars.lock();
  physics::Variables& roe_vars = *m_roe_vars.lock();
  // Compute left and right properties
  sol_vars.compute_properties(coord,left,grads,*p_left);
  sol_vars.compute_properties(coord,right,grads,*p_right);

  // Compute the Roe averaged properties
  // Roe-average = standard average of the Roe-parameter vectors
  roe_vars.compute_variables(*p_left,  roe_left );
  roe_vars.compute_variables(*p_right, roe_right);
  roe_avg = 0.5*(roe_left+roe_right);                // Roe-average is result
  roe_vars.compute_properties(coord, roe_avg, grads, *p_avg);

  // Compute absolute jacobian using Roe averaged properties
  sol_vars.flux_jacobian_eigen_structure(*p_avg,normal,right_eigenvectors,left_eigenvectors,eigenvalues);
  RealMatrix& abs_jacobian = right_eigenvectors;
  abs_jacobian *= eigenvalues.cwiseAbs().asDiagonal();
  abs_jacobian *= left_eigenvectors;

  // Compute left and right fluxes
  sol_vars.flux(*p_left , f_left);
  sol_vars.flux(*p_right, f_right);

  // Compute flux at interface composed of central part and upwind part
  central_flux  = f_left*normal;
  central_flux += f_right*normal;
  central_flux *= 0.5;

  upwind_flux  = abs_jacobian*(right-left);
  upwind_flux *= 0.5;
  flux  = central_flux;
  flux -= upwind_flux;
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
} // cf3
