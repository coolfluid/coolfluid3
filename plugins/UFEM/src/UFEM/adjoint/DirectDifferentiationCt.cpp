// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "DirectDifferentiationCt.hpp"

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
#include "../SurfaceIntegral.hpp"
#include "../NavierStokesSpecializations.hpp"
#include "../Tags.hpp"

namespace cf3 {
namespace UFEM {
namespace adjoint {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;
using boost::proto::lit;

ComponentBuilder < DirectDifferentiationCt, LSSActionUnsteady, LibUFEMAdjoint > DirectDifferentiationCt_Builder;

DirectDifferentiationCt::DirectDifferentiationCt(const std::string& name) :
  LSSActionUnsteady(name),
  u("Velocity", "navier_stokes_solution"),
  SensU("SensU","sensitivity_solution"),
  SensP("SensP","sensitivity_solution"),
  nu_eff("EffectiveViscosity", "navier_stokes_viscosity"),
  density_ratio("density_ratio", "density_ratio"),
  g("Force", "body_force"),
  rho("density"),
  nu("kinematic_viscosity")
  // J("sensitivity","sensitivity_derivative")
{
  const std::vector<std::string> restart_field_tags = boost::assign::list_of("navier_stokes_solution")("adjoint_solution")("adj_linearized_velocity")("navier_stokes_viscosity")("sensitivity_solution");
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
    .attach_trigger(boost::bind(&DirectDifferentiationCt::trigger_ct, this)) // the function trigger_ct is called whenever the ct option is changed
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

  set_solution_tag("sensitivity_solution");

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

DirectDifferentiationCt::~DirectDifferentiationCt()
{
}


void DirectDifferentiationCt::trigger_assembly()
{
  Uint Nt = 0;
  m_assembly->clear();
  m_update->clear();
  m_assembly->add_component(create_proto_action
  (
    "AdjointAssembly",
    elements_expression
    (
      boost::mpl::vector2<mesh::LagrangeP1::Tetra3D,
          mesh::LagrangeP1::Triag2D
          >(),
      group
      (
        _A = _0, _T = _0, _a = _0,
          compute_tau.apply(u, nu_eff, lit(dt()), lit(tau_ps), lit(tau_su), lit(tau_bulk)),
          element_quadrature
          (
            _A(SensP    , SensU[_i]) += transpose(N(SensP)) * nabla(SensU)[_i] + tau_ps * transpose(nabla(SensP)[_i]) * u*nabla(SensU), // Standard continuity + PSPG for advection
            _A(SensP    , SensP)     += tau_ps * transpose(nabla(SensP)) * nabla(SensP), // Continuity, PSPG
            _A(SensU[_i], SensU[_i]) += nu_eff * transpose(nabla(SensU)) * nabla(SensU) + transpose(N(u) + tau_su*u*nabla(SensU)) * u*nabla(SensU), // Diffusion + advection
            _A(SensU[_i], SensP)     += transpose(N(SensU) + tau_su*u*nabla(SensU)) * nabla(SensP)[_i], // Pressure gradient (standard and SUPG)
            _A(SensU[_i], SensU[_j]) += transpose(tau_bulk*nabla(SensU)[_i])*nabla(SensU)[_j] + transpose(N(SensU) + tau_su*u*nabla(SensU))*N(SensU)*_row(nabla(u)*nodal_values(u), _j)[_i],// + partial(u[_i],_j), // *(nabla(u)*partial(u[_i],_j)*transpose(nabla(u))),
            // _a[SensU[_i]] += transpose(N(SensU) + tau_su*u*nabla(SensU)) * g[_i] /* * normal[_i] */ * density_ratio /(lit(m_a[Nt])*(lit(1.0)-lit(m_a[Nt]))),

            _T(SensP    , SensU[_i]) += tau_ps * transpose(nabla(SensP)[_i]) * N(SensU), // Time, PSPG
            _T(SensU[_i], SensU[_i]) += transpose(N(SensU) + tau_su*u*nabla(SensU)) * N(SensU)
          ),
        system_rhs += -_A * _x + _a,
        _A(SensP) = _A(SensP) / theta,
        system_matrix += invdt() * _T + theta * _A
      )
    )
  ));

  // Uint Nt = 0;
  for (auto&& region : m_actuator_regions)
  {
    auto region_action = create_proto_action(region->name(), 
    elements_expression
    (
      boost::mpl::vector</*mesh::LagrangeP1::Triag3D, */mesh::LagrangeP1::Line2D>(),
      group
      (
        _A(SensU) = _0, _A(SensP) = _0, _a[SensU] = _0, _a[SensP] = _0,
        //compute_tau.apply(u, nu_eff, lit(dt()), lit(tau_ps), lit(tau_su), lit(tau_bulk)),
        element_quadrature
        (
            _a[SensU[_i]] += - transpose(N(SensU)) * normal[_i] * lit(0.5) * u[0] * u[0] / lit(m_th) * density_ratio
        ),
        // element_quadrature(_A(SensU[_i], SensU[_i]) += transpose(N(SensU))*N(SensU)*u[_i]* lit(4) * lit(m_a[Nt])/(lit(1)-lit(m_a[Nt]))/ lit(m_th)*density_ratio * normal[_i]), // integrate
        system_rhs += _a
        //system_matrix += theta * _A
      )
    ));
    m_assembly->add_component(region_action);
    region_action->options().set("regions", std::vector<common::URI>({region->uri()}));
    region_action->options().option("regions").add_tag("norecurse");
    Nt+=1;
  }

  //   for(auto&& region : m_actuator_regions)
  // {
  //     auto region_action = create_proto_action(region->name(), elements_expression(boost::mpl::vector2<      mesh::LagrangeP1::Line2D,
  //         mesh::LagrangeP1::Triag3D>(), group(
  //                                                      // set element vector to zero Line2D Triag3D
	// 												  _A = _0, _A = _0, _a = _0,

  //                           element_quadrature( _a[SensU[_i]] += transpose(N(SensU) /*+ tau_su*u*nabla(SensU)*/) * normal[_i] * density_ratio  * lit(2) * u[_i] * u[_i] / (lit(1) - lit(m_a[Nt])) / (lit(1) - lit(m_a[Nt]))
  //                           ), //* density_ratio * normal[_i]
  //                           // element_quadrature(_A(SensU[_i], SensU[_i]) += transpose(N(SensU))*N(SensU)*u[_i]* lit(4) * lit(m_a[Nt])/(lit(1)-lit(m_a[Nt]))/ lit(m_th)*density_ratio * normal[_i]), // integrate
  //                           system_rhs +=-_A * _x + _a, // update global system RHS with element vector
	// 												  system_matrix += theta * _A
  //                                                  )));
  //     m_assembly->add_component(region_action);
  //     region_action->options().set("regions", std::vector<common::URI>({region->uri()}));
  //     region_action->options().option("regions").add_tag("norecurse");
  //     Nt+=1;
  // }

  m_update->add_component(create_proto_action("Update", 
    nodes_expression(
      group
        (
        SensU += solution(SensU),
        SensP += solution(SensP)
      )
    )
  ));

  if(is_not_null(m_physical_model))
  {
    configure_option_recursively(solver::Tags::physical_model(), m_physical_model);
  }

  // Ensure the initial condition field list gets updated
  if(is_not_null(m_initial_conditions))
    m_initial_conditions->options().set("field_tag", solution_tag());
}



//On region set actuator disk components
void DirectDifferentiationCt::on_regions_set()
{
	if(m_updating == true){
		return;
	}
  if(m_loop_regions.empty())
  {
    return; // no regions -> do nothing
  }
  m_updating = true;
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

void DirectDifferentiationCt::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  m_initial_conditions = initial_conditions.create_initial_condition(solution_tag());
}


void DirectDifferentiationCt::trigger_ct()
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

} // adjoint
} // UFEM
} // cf3
