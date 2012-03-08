#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"

#include "RiemannSolvers/LaxFriedrich.hpp"


namespace cf3 {
namespace RiemannSolvers {

using namespace common;
using namespace physics;

common::ComponentBuilder < LaxFriedrich, RiemannSolver, LibRiemannSolvers > LaxFriedrich_Builder;

////////////////////////////////////////////////////////////////////////////////

LaxFriedrich::LaxFriedrich ( const std::string& name ) : RiemannSolver(name)
{
  options().option("physical_model").attach_trigger( boost::bind( &LaxFriedrich::trigger_physical_model, this) );
}

////////////////////////////////////////////////////////////////////////////////

LaxFriedrich::~LaxFriedrich()
{
}

////////////////////////////////////////////////////////////////////////////////

void LaxFriedrich::trigger_physical_model()
{
  coord.resize(physical_model().ndim());
  grads.resize(physical_model().neqs(),physical_model().ndim());
  p_left  = physical_model().create_properties();
  p_right = physical_model().create_properties();
  p_avg   = physical_model().create_properties();

  f_left.resize(physical_model().neqs(),physical_model().ndim());
  f_right.resize(physical_model().neqs(),physical_model().ndim());

  eigenvalues.resize(physical_model().neqs());
  eigenvalues_left.resize(physical_model().neqs());
  eigenvalues_right.resize(physical_model().neqs());
  right_eigenvectors.resize(physical_model().neqs(),physical_model().neqs());
  left_eigenvectors.resize(physical_model().neqs(),physical_model().neqs());
  abs_jacobian.resize(physical_model().neqs(),physical_model().neqs());

//  central_flux.resize(physical_model().neqs());
  upwind_flux.resize(physical_model().neqs());

  // Try to configure solution_vars automatically
  if (is_null(m_solution_vars))
  {
    if (Handle<Component> found_solution_vars = find_component_ptr_recursively_with_name(physical_model(),"solution_vars"))
    {
      options().
      configure_option("solution_vars",found_solution_vars->uri());
    }
    else
    {
      CFwarn << "LaxFriedrich RiemannSolver " << uri().string() << " could not auto-config \"solution_vars\".\n"
             << "Reason: component with name \"solution_vars\" not found in ["<<physical_model().uri().string() << "]\n"
             << "Configure manually" << CFendl;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void LaxFriedrich::compute_interface_flux(const RealVector& left, const RealVector& right, const RealVector& coords, const RealVector& normal,
                                     RealVector& flux)
{
  physics::Variables& sol_vars = *m_solution_vars;
  // Compute left and right properties
  sol_vars.compute_properties(coord,left,grads,*p_left);
  sol_vars.compute_properties(coord,right,grads,*p_right);

  // Compute left and right fluxes
  sol_vars.flux(*p_left , f_left);
  sol_vars.flux(*p_right, f_right);

  // Compute flux at interface
  sol_vars.flux_jacobian_eigen_values(*p_left, normal, eigenvalues_left);
  sol_vars.flux_jacobian_eigen_values(*p_right, normal, eigenvalues_right);

  eigenvalues = eigenvalues_left;
  eigenvalues += eigenvalues_right;
  eigenvalues *= 0.5;

  RealMatrix absA = eigenvalues_left.cwiseAbs().asDiagonal();
  absA += eigenvalues_right.cwiseAbs().asDiagonal();
  absA *= 0.5;

  flux = 0.5*(f_left + f_right)-absA*(right-left);

}

////////////////////////////////////////////////////////////////////////////////

void LaxFriedrich::compute_interface_flux_and_wavespeeds(const RealVector& left, const RealVector& right, const RealVector& coords, const RealVector& normal,
                                                RealVector& flux, RealVector& wave_speeds)
{
  compute_interface_flux(left,right,coords,normal,flux);
  wave_speeds = eigenvalues;
}

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // cf3
