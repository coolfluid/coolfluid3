// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Adjoint.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <cmath>
#include <boost/mpl/back_inserter.hpp>
#include <boost/mpl/copy.hpp>

#include "common/Component.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PropertyList.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/ElementGradDiv.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "solver/actions/Proto/SurfaceIntegration.hpp"
#include "../NavierStokesSpecializations.hpp"
#include "../Tags.hpp"

namespace cf3 {
namespace UFEM {
namespace adjoint{

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;
using boost::proto::lit;

ComponentBuilder < Adjoint, LSSActionUnsteady, LibUFEMAdjoint > Adjoint_Builder;

Adjoint::Adjoint(const std::string& name) :
  LSSActionUnsteady(name),
  u("Velocity", "navier_stokes_solution"),
  ka("ka", "keAdjoint_solution"),
  epsilona("epsilona", "keAdjoint_solution"),
  k("k", "ke_solution"),
  epsilon("epsilon", "ke_solution"),
  U("AdjVelocity", "adjoint_solution"),
  q("AdjPressure", "adjoint_solution"),
  nu_eff("EffectiveViscosity", "navier_stokes_viscosity"),
  density_ratio("density_ratio", "density_ratio"),
  g("Force", "body_force"),
  rho("density"),
  nu("kinematic_viscosity")
{
  const std::vector<std::string> restart_field_tags = boost::assign::list_of("navier_stokes_solution")("adjoint_solution")("adj_linearized_velocity")("navier_stokes_viscosity");
  properties().add("restart_field_tags", restart_field_tags);

  options().add("supg_type", compute_tau.data.op.supg_type_str)
    .pretty_name("SUPG Type")
    .description("Type of computation for the stabilization coefficients.")
    .link_to(&(compute_tau.data.op.supg_type_str))
    .attach_trigger(boost::bind(&ComputeTauImpl::trigger_supg_type, &compute_tau.data.op));

  options().add("u_ref", compute_tau.data.op.u_ref)
    .pretty_name("Reference velocity")
    .description("Reference velocity for the CF2 SUPG method")
    .link_to(&(compute_tau.data.op.u_ref));

  options().add("ct", m_ct)
    .pretty_name("trust coefficient")
    .description("trust coefficient")
    .attach_trigger(boost::bind(&Adjoint::trigger_ct, this)) // the function trigger_ct is called whenever the ct option is changed
    .mark_basic();

  options().add("a", m_a)
    .pretty_name("Axial induction coefficient")
    .description("Axial induction coefficient")
    .attach_trigger(boost::bind(&Adjoint::trigger_a, this))
    .mark_basic();

  options().add("area", m_area)
    .pretty_name("area of the disk")
    .description("area of the disk")
    .link_to(&m_area)
    .mark_basic();

  options().add("turbulence", m_turbulence)
    .pretty_name("Frozen turbulence parameter")
    .description("enable of disable forzen turbulence assumption")
    .link_to(&m_turbulence)
    .mark_basic();

 options().add("th", m_th)
    .pretty_name("Mesh finesse")
    .description("Mesh finesse")
    .link_to(&m_th)
    .mark_basic();

  options().add("theta", theta)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&theta);

   options().add("result", Real())
      .pretty_name("Result")
      .description("Result of the integration (read-only)")
      .mark_basic();

  options().option("regions").add_tag("norecurse");

  set_solution_tag("adjoint_solution");

  // This ensures that the linear system matrix is reset to zero each timestep
  create_component<math::LSS::ZeroLSS>("ZeroLSS")->options().set("reset_solution", false);

  // Container for the assembly actions. Will be filled depending on the value of options, such as using specializations or not
  m_assembly = create_component<solver::ActionDirector>("Assembly");

  // Boundary conditions
  Handle<BoundaryConditions> bc =  create_component<BoundaryConditions>("BoundaryConditions");
  bc->mark_basic();
  bc->set_solution_tag(solution_tag());

  // Solution of the LSS
  create_component<math::LSS::SolveLSS>("SolveLSS");

  // Update of the solution
  m_update = create_component<solver::ActionDirector>("UpdateActions");


  trigger_assembly();
}

Adjoint::~Adjoint()
{
}


void Adjoint::trigger_assembly()
{
  m_assembly->clear();
  m_update->clear();
  m_assembly->add_component(create_proto_action
  (
    "AdjointAssembly",
    elements_expression
    (
      boost::mpl::vector2<
          mesh::LagrangeP1::Triag2D,
          mesh::LagrangeP1::Tetra3D
          >(),
      group
      (
        _A = _0, _T = _0, _a = _0,
          compute_tau.apply(u, nu_eff, lit(dt()), lit(tau_ps), lit(tau_su), lit(tau_bulk)),
          element_quadrature
          (
                  _A(q    , U[_i]) += transpose(N(q) /*- tau_ps*u*nabla(q)*0.5*/) * nabla(U)[_i] - tau_ps * transpose(nabla(q)[_i]) * u*nabla(U), // Standard continuity + PSPG for advection
                  _A(q    , q)     += tau_ps * transpose(nabla(q)) * nabla(q), // Continuity, PSPG
                  _A(U[_i], U[_i]) += nu_eff * transpose(nabla(U)) * nabla(U) - transpose(N(u) - tau_su*u*nabla(U)) * u*nabla(U), // Diffusion + advection
                  _A(U[_i], q)     += transpose(N(U) - tau_su*u*nabla(U)) * nabla(q)[_i], // Pressure gradient (standard and SUPG)
                  _A(U[_i], U[_j]) += transpose(tau_bulk*nabla(U)[_i])* nabla(U)[_j]-transpose(N(U) - tau_su*u*nabla(U)) * u[_j] * nabla(U)[_i], // Bulk viscosity + additional adjoint advection term
                                      //+ 0.5*u[_i]*(N(U) - tau_su*u*nabla(U)) * nabla(U)[_j], //  skew symmetric part of advection (standard +SUPG)
                  _T(q    , U[_i]) += tau_ps * transpose(nabla(q)[_i]) * N(U), // Time, PSPG
                  _T(U[_i], U[_i]) += transpose(N(U) - tau_su*u*nabla(U)) * N(U), // Time, standard and SUPG
                  _a[U[_i]] += /* -transpose(N(U) - tau_su*u*nabla(U)) * 3 * g[_i] * density_ratio */ + 
                            m_turbulence*(-(transpose(N(U) - tau_su*u*nabla(U))*ka*gradient(k)[_i]) - (transpose(N(U) - tau_su*u*nabla(U))*epsilona*gradient(epsilon)[_i])
                                            +(2*((ka*k/epsilon)+(epsilona*m_c_epsilon_1))*k*m_c_mu* transpose(nabla(U)) *_col(partial(u[_i],_j)+partial(u[_j],_i),_i)))
          ),
        system_rhs += -_A * _x + _a,
        _A(q) = _A(q) / theta,
        system_matrix += invdt() * _T + theta * _A
      )
    )
  ));
  Uint Nt = 0.;
  for(auto&& region : m_actuator_regions)
  {
    // CFinfo << "Test assembly" << CFendl;
      auto region_action = create_proto_action(region->name(), elements_expression(boost::mpl::vector2<      mesh::LagrangeP1::Line2D,
          mesh::LagrangeP1::Triag3D>(), group(
                                                       // set element vector to zero Line2D Triag3D
													  _A(q) = _0, _A(U) = _0, _a[U] = _0, _a[q] = _0,

                            element_quadrature
                            (
                              _A(U[_i], U[_i]) += transpose(N(U))*N(U)*u[_i]* lit(4) * lit(m_a[Nt])/(lit(1)-lit(m_a[Nt]))/ lit(m_th)*density_ratio * normal[_i],
                              _a[U[_i]] += transpose(N(U)) * lit(6.0) * u[0] * u[0] * lit(m_a[Nt])/(1 - lit(m_a[Nt])) * normal[_i] * density_ratio
                              //_a[U[_i]] += transpose(N(U)) * -3 * g[_i] * normal[_i] * density_ratio
                            ), // integrate
                            system_rhs +=-_A * _x + _a, // update global system RHS with element vector
													  system_matrix += theta * _A
                                                   )));
      m_assembly->add_component(region_action);
      region_action->options().set("regions", std::vector<common::URI>({region->uri()}));
      region_action->options().option("regions").add_tag("norecurse");
      Nt+=1;
  }

  m_update->add_component(create_proto_action("Update", nodes_expression(group
  (
    U += solution(U),
    q += solution(q)
  ))));

  if(is_not_null(m_physical_model))
  {
    configure_option_recursively(solver::Tags::physical_model(), m_physical_model);
  }

  // Ensure the initial condition field list gets updated
  if(is_not_null(m_initial_conditions))
    m_initial_conditions->options().set("field_tag", solution_tag());
}

//On region set actuator disk components
void Adjoint::on_regions_set()
{  
	if(m_updating == true){
		return;
	}
  if(m_loop_regions.empty())
  {
    return; // no regions -> do nothing
  }
  m_updating = true;
  // CFinfo << "Test on region set" << m_loop_regions.size() << CFendl;
  // Put all regions except the first one in m_actuator_regions
  if (m_first_call == true)
  {
    m_actuator_regions.clear();
    m_actuator_regions.insert(m_actuator_regions.end(), m_loop_regions.begin()+1, m_loop_regions.end());
  }
  // Remove all except the first region from m_loop_regions
  m_loop_regions.resize(1);

  trigger_assembly();
  LSSActionUnsteady::on_regions_set();
  m_updating = false;
  m_first_call = false;
}

void Adjoint::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  m_initial_conditions = initial_conditions.create_initial_condition(solution_tag());
}
void Adjoint::execute()
{
	LSSActionUnsteady::execute();
	Uint Nt1 = 0.;
	  for(auto&& region : m_actuator_regions)
  {
    // Real part1 = 0.0;
    // surface_integral(part1, std::vector<Handle<mesh::Region>>({region}), 1);
    // CFinfo << "Disk area = " << part1 << CFendl;
    // CFinfo << "Test actuator" << CFendl;
	 FieldVariable<0, VectorField> U("AdjVelocity", "adjoint_solution");
     m_U_mean_disk = 0;
     surface_integral(m_U_mean_disk, std::vector<Handle<mesh::Region>>({region}), _abs((U*normal)[0]));
     m_U_mean_disk /= m_area;
	 options().set("result",m_U_mean_disk);
     CFinfo << "Mean adjoint Velocity " << m_U_mean_disk << ", a: " << m_a[Nt1] << CFendl;
	 Nt1 +=1;
  }
}

void Adjoint::trigger_ct()
{
  auto new_ct = options().value<std::vector<Real>>("ct");
  bool size_changed = new_ct.size() != m_ct.size(); // If the size changed, assembly needs to be ran again

  // Resize if needed
  m_ct.resize(new_ct.size());
  m_a.resize(new_ct.size());

  // Copy ct values
  std::copy(new_ct.begin(), new_ct.end(), m_ct.begin());

  // Update a values
  std::transform(new_ct.begin(), new_ct.end(), m_a.begin(), [](Real ct) { return (1.-std::sqrt(1-ct))/2.; });

  if(size_changed)
  {
    trigger_assembly();
  }
}

void Adjoint::trigger_a()
{
  auto new_a = options().value<std::vector<Real>>("a");
  bool size_changed = new_a.size() != m_a.size();

  m_a.resize(new_a.size());

  std::copy(new_a.begin(), new_a.end(), m_a.begin());

  if (size_changed)
  {
    trigger_assembly();
  }
}

} // adjoint
} // UFEM
} // cf3
