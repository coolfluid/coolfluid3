// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <cmath>
#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/EventHandler.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "ActuatorDiskCt.hpp"

#include "solver/actions/Proto/SurfaceIntegration.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include <iomanip>

namespace cf3
{

namespace UFEM
{

namespace adjoint
{

using namespace solver::actions::Proto;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ActuatorDiskCt, common::Action, LibUFEMAdjoint > ActuatorDiskCt_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

using boost::proto::lit;

ActuatorDiskCt::ActuatorDiskCt(const std::string& name) :
  UnsteadyAction(name),
  rhs(options().add("lss", Handle<math::LSS::System>())
    .pretty_name("LSS")
    .description("The linear system for which the boundary condition is applied")),
  system_matrix(options().option("lss"))
{
  options().add("u_in", m_u_in)
    .pretty_name("Velocityin")
    .description("Inlet velocity")
    .link_to(&m_u_in)
    .mark_basic(); // is this is enabled, the option can be accessed directly from Python, otherwise .options is needed

  options().add("area", m_area)
    .pretty_name("Area")
    .description("Area of the disk")
    .link_to(&m_area)
    .mark_basic(); // is this is enabled, the option can be accessed directly from Python, otherwise .options is needed
  options().add("ct", m_ct)
    .pretty_name("Ct")
    .description("Trust coefficient of the disk")
    .link_to(&m_ct)
    .mark_basic(); // if ct < 0 --> a in function of u_mean_disk

  options().add("th", m_th)
    .pretty_name("Thickness")
    .description("Thickness of the disk")
    .link_to(&m_th)
    .mark_basic();
	
  options().add("result", Real())
      .pretty_name("Result")
      .description("Result of the integration (read-only)")
      .mark_basic();

  
  // The component that  will set the force
  create_static_component<ProtoAction>("SetForce")->options().option("regions").add_tag("norecurse");

  // Initialize the expression
  trigger_setup();
}

ActuatorDiskCt::~ActuatorDiskCt()
{
}

void ActuatorDiskCt::on_regions_set()
{
  auto regions = options()["regions"].value<std::vector<common::URI>>();

  // Set the regions when the option is set
  get_child("SetForce")->options().set("regions", std::vector<common::URI>({regions[0]}));
}

void ActuatorDiskCt::trigger_setup()
{
  Handle<ProtoAction> set_force(get_child("SetForce"));

  FieldVariable<0, VectorField> f("Force", "body_force");
  FieldVariable<1, VectorField> u("Velocity", "navier_stokes_solution");
  set_force->set_expression(nodes_expression
  (
    group
    (
      f[0] = lit(m_f)
     )
  ));
}

void ActuatorDiskCt::execute()
{
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  m_u_mean_disk = 0;
  const auto pow2 = make_lambda([](Real x ){
	  return x*x;
	    });
  const auto pow3 = make_lambda([](Real x ){
	  return x*x*x;
  });

  surface_integral(m_u_mean_disk, std::vector<Handle<mesh::Region>>({m_loop_regions[1]}), _abs((u*normal)[0]));

  m_u_mean_disk /= m_area;

  m_f = -0.5 * m_ct * m_u_mean_disk * m_u_mean_disk / m_th;//(m_dt * m_u_mean_disk);
  // CFinfo << std::setprecision(20) <<"force set to " << m_f << ", a: " << m_a << "m_u_mean_disk :" << m_u_mean_disk <<  " pow2 " << m_u_mean_disk2 << " pow3 " << m_u_mean_disk3 << CFendl;
  options().set("result", m_u_mean_disk);
  Handle<ProtoAction> set_force(get_child("SetForce"));
  set_force->execute();
}

} // namespace adjoint

} // namespace UFEM

} // namespace cf3
