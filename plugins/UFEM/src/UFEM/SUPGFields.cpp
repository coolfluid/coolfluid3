// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PE/Comm.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/EventHandler.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"
#include "mesh/LagrangeP0/Triag.hpp"
#include "mesh/LagrangeP0/Hexa.hpp"
#include "mesh/LagrangeP0/Tetra.hpp"
#include "mesh/LagrangeP0/Prism.hpp"

#include "SUPG.hpp"
#include "SUPGFields.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/Tags.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;
using boost::proto::lit;

common::ComponentBuilder < SUPGFields, common::Action, LibUFEM > SUPGFields_Builder;

SUPGFields::SUPGFields(const std::string& name) :
  ProtoAction(name)
{
  options().add(solver::Tags::time(), m_time)
    .pretty_name("Time")
    .description("Component that keeps track of time for this simulation")
    .attach_trigger(boost::bind(&SUPGFields::trigger_time, this))
    .link_to(&m_time);
    
  options().add("alpha_ps", compute_tau.data.op.alpha_ps)
    .pretty_name("alpha_ps")
    .description("Constant to multiply the PSPG parameter with.")
    .link_to(&(compute_tau.data.op.alpha_ps));
      
  options().add("alpha_su", compute_tau.data.op.alpha_su)
    .pretty_name("alpha_su")
    .description("Constant to multiply the SUPG parameter with.")
    .link_to(&(compute_tau.data.op.alpha_su));
      
  options().add("alpha_bu", compute_tau.data.op.alpha_bu)
    .pretty_name("alpha_bu")
    .description("Constant to multiply the Bulk parameter with.")
    .link_to(&(compute_tau.data.op.alpha_bu));
    
  options().add("supg_type", compute_tau.data.op.supg_type_str)
    .pretty_name("SUPG Type")
    .description("Type of computation for the stabilization coefficients.")
    .link_to(&(compute_tau.data.op.supg_type_str))
    .attach_trigger(boost::bind(&ComputeTauImpl::trigger_supg_type, &compute_tau.data.op));
    
  options().add("c1", compute_tau.data.op.c1)
    .pretty_name("c1")
    .description("Constant adjusting the time part of SUPG in the metric tensor formulation")
    .link_to(&(compute_tau.data.op.c1));
    
  options().add("c2", compute_tau.data.op.c2)
    .pretty_name("c2")
    .description("Constant adjusting the time part of SUPG in the metric tensor formulation")
    .link_to(&(compute_tau.data.op.c2));
    
  FieldVariable<0, VectorField> u_adv("AdvectionVelocity", "linearized_velocity");
  FieldVariable<1, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");
  FieldVariable<2, ScalarField> su_fd("tau_su", "supg", mesh::LagrangeP0::LibLagrangeP0::library_namespace());
  FieldVariable<3, ScalarField> bulk_fd("tau_bu", "supg", mesh::LagrangeP0::LibLagrangeP0::library_namespace());
  
  set_expression(elements_expression
  (
    boost::mpl::vector10<mesh::LagrangeP0::Triag, mesh::LagrangeP1::Triag2D, mesh::LagrangeP0::Quad, mesh::LagrangeP1::Quad2D, mesh::LagrangeP0::Hexa, mesh::LagrangeP1::Hexa3D, mesh::LagrangeP0::Tetra, mesh::LagrangeP1::Tetra3D, mesh::LagrangeP0::Prism, mesh::LagrangeP1::Prism3D>(),
    group
    (
      compute_tau.apply(u_adv, nu_eff, lit(m_dt), lit(tau_ps), lit(tau_su), lit(tau_bu)),
      su_fd = lit(tau_su),
      bulk_fd = lit(tau_bu)
    )
  ));

  FieldVariable<2, VectorField> u("Velocity", "navier_stokes_u_solution");
  FieldVariable<3, VectorField> u1("AdvectionVelocity1", "linearized_velocity");
  FieldVariable<4, ScalarField> p("Pressure", "navier_stokes_p_solution");
  FieldVariable<5, VectorField> supg_a("supg_a", "supg_terms");
  FieldVariable<6, VectorField> supg_t("supg_t", "supg_terms");
  FieldVariable<7, VectorField> bulk("bulk", "supg_terms");
  FieldVariable<8, VectorField> viscous("viscous", "supg_terms");
  FieldVariable<9, VectorField> advection("advection", "supg_terms");

  static boost::proto::terminal< ElementVector< boost::mpl::int_<1> > >::type const _b = {};
  static boost::proto::terminal< ElementVector< boost::mpl::int_<2> > >::type const _c = {};
  static boost::proto::terminal< ElementVector< boost::mpl::int_<3> > >::type const _d = {};
  static boost::proto::terminal< ElementVector< boost::mpl::int_<4> > >::type const _e = {};

  m_supg_terms = create_static_component<ProtoAction>("SUPGTerms");
  m_supg_terms->set_expression(elements_expression
  (
    //boost::mpl::vector5<mesh::LagrangeP1::Triag2D, mesh::LagrangeP1::Quad2D, mesh::LagrangeP1::Hexa3D, mesh::LagrangeP1::Tetra3D, mesh::LagrangeP1::Prism3D>(),
    boost::mpl::vector2<mesh::LagrangeP1::Quad2D, mesh::LagrangeP1::Hexa3D>(),
    group
    (
      compute_tau.apply(u_adv, nu_eff, lit(m_dt), lit(tau_ps), lit(tau_su), lit(tau_bu)),
      group(_A(u) = _0, _a[u] = _0, _b[u] = _0, _c[u] = _0, _d[u] = _0, _e[u] = _0),
      element_quadrature
      (
        _a[u[_i]] += transpose(tau_su*u_adv*nabla(u)) * (nabla(p)[_i] * nodal_values(p) + u_adv*nabla(u) * transpose(transpose(nodal_values(u))[_i])),
        _a[u[_i]] += 0.5*transpose(u_adv[_i]*(tau_su*u_adv*nabla(u))) * nabla(u)[_j] * transpose(transpose(nodal_values(u))[_j]),
        _b[u[_i]] += transpose(tau_su*u_adv*nabla(u)) * N(u) * transpose(transpose(nodal_values(u))[_i] - transpose(nodal_values(u1))[_i]),
        _c[u[_i]] += transpose(tau_bu*nabla(u)[_i]) * nabla(u)[_j] * transpose(transpose(nodal_values(u))[_j]),
        _d[u[_i]] += nu_eff * transpose(nabla(u)) * nabla(u) * transpose(transpose(nodal_values(u))[_i]),
        _e[u[_i]] += transpose(N(u)) * u_adv*nabla(u) * transpose(transpose(nodal_values(u))[_i])
      ),
      supg_a += _a / volume,
      supg_t += _b /(volume * m_dt),
      bulk += _c / volume,
      viscous += _d / volume,
      advection += _e / volume
    )
  ));

  m_zero_field = create_static_component<ProtoAction>("ZeroField");
  m_zero_field->set_expression(nodes_expression(group
  (
    supg_a[_i] = 0.,
    supg_t[_i] = 0.,
    bulk[_i] = 0.,
    viscous[_i] = 0.,
    advection[_i] = 0.
  )));
}

SUPGFields::~SUPGFields()
{
}

void SUPGFields::execute()
{
  m_zero_field->execute();
  m_supg_terms->execute();
  cf3::solver::actions::Proto::ProtoAction::execute();
}

void SUPGFields::on_regions_set()
{
  cf3::solver::Action::on_regions_set();
  m_zero_field->options().set("regions", options().option("regions").value());
  m_supg_terms->options().set("regions", options().option("regions").value());
}

void SUPGFields::trigger_time()
{
  if(is_null(m_time))
      return;

  m_time->options().option("time_step").attach_trigger(boost::bind(&SUPGFields::trigger_timestep, this));
  trigger_timestep();
}

void SUPGFields::trigger_timestep()
{
  m_dt = m_time->dt();
  CFdebug << "Set SUPGFields time step to " << m_dt << CFendl;
}

} // namespace UFEM

} // namespace cf3
