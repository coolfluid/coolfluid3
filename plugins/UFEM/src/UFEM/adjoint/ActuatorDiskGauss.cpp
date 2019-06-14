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
#include "math/Consts.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "ActuatorDiskGauss.hpp"

#include "solver/actions/Proto/SurfaceIntegration.hpp"
#include "solver/actions/Proto/VolumeIntegration.hpp"
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

common::ComponentBuilder < ActuatorDiskGauss, common::Action, LibUFEMAdjoint > ActuatorDiskGauss_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

using boost::proto::lit;

ActuatorDiskGauss::ActuatorDiskGauss(const std::string& name) :
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

  options().add("delta", m_delta)
    .pretty_name("delta")
    .description("Parameter of the Gaussian filter")
    .link_to(&m_delta)
    .mark_basic();
	
  options().add("result", Real())
    .pretty_name("Result")
    .description("Result of the integration (read-only)")
    .mark_basic();

  
  // The component that  will set the force
  create_static_component<ProtoAction>("SetForce")->options().option("regions").add_tag("norecurse");
  create_static_component<ProtoAction>("SetCt")->options().option("regions").add_tag("norecurse");
  create_static_component<ProtoAction>("SetuDisk")->options().option("regions").add_tag("norecurse");

  create_static_component<ProtoAction>("SetFilter")->options().option("regions").add_tag("norecurse");

  // Initialize the expression
  trigger_setup();
  trigger_set_expression();
}

ActuatorDiskGauss::~ActuatorDiskGauss()
{
}

void ActuatorDiskGauss::on_regions_set()
{
  auto regions = options()["regions"].value<std::vector<common::URI>>();

  // Set the regions when the option is set
  get_child("SetForce")->options().set("regions", std::vector<common::URI>({regions[0]}));
  get_child("SetCt")->options().set("regions", std::vector<common::URI>({regions[0]}));
  get_child("SetuDisk")->options().set("regions", std::vector<common::URI>({regions[0]}));
  get_child("SetFilter")->options().set("regions", std::vector<common::URI>({regions[0]}));
}

void ActuatorDiskGauss::trigger_set_expression()
{
  //FieldVariable<2, VectorField> u("Velocity", options().value<std::string>("velocity_tag"));
  do_set_expressions(*Handle<ProtoAction>(get_child("SetFilter")));//, *Handle<ProtoAction>(get_child("UpdateNut")), u);
}

void ActuatorDiskGauss::do_set_expressions(solver::actions::Proto::ProtoAction& set_filter)
{
  const auto m_gaussIntegral = make_lambda([&] (Real x) {
      return 0.5 * tanh(39.0*x/2.0 - 111.0/2.0 * atan(35.0*x / 111.0));
  });

  FieldVariable<0, ScalarField> R("GaussFilter", "actuator_disk");
  set_filter.set_expression(nodes_expression
    (
        group
        (
            R = lit(m_prefix) * ((m_gaussIntegral(lit(m_prefix) * (coordinates[1] + lit(m_area)/lit(2.0)))) - 
                     (m_gaussIntegral(lit(m_prefix) * (coordinates[1] - lit(m_area)/lit(2.0))))) * 
                     _exp(-lit(6.0)*(coordinates[0] * coordinates[0]) / (lit(m_delta) * lit(m_delta))) 
            //R = m_delta
        )
    )
    );
}

void ActuatorDiskGauss::trigger_setup()
{
  Handle<ProtoAction> set_force(get_child("SetForce"));

  FieldVariable<0, VectorField> f("Force", "body_force");
  FieldVariable<1, VectorField> u("Velocity", "navier_stokes_solution");
  FieldVariable<2, ScalarField> Ct("ThrustCoefficient", "actuator_disk");
  FieldVariable<3, VectorField> uDisk("MeanDiskSpeed", "actuator_disk");
  FieldVariable<4, ScalarField> R("GaussFilter", "actuator_disk");

  set_force->set_expression(nodes_expression
  (
    group
    (
      f[0] = lit(m_f)*R
     )
  ));
  Handle<ProtoAction> set_ct(get_child("SetCt"));
  Handle<ProtoAction> set_uDisk(get_child("SetuDisk"));

  set_ct->set_expression(nodes_expression
  (
    group
    (
      Ct = lit(m_ct)
    )
  ));
  set_uDisk->set_expression(nodes_expression
  (
    group
    (
      uDisk[0] = lit(m_u_mean_disk)
    )
  ));
}

void ActuatorDiskGauss::execute()
{
  FieldVariable<0, VectorField> f("Force", "body_force");
  FieldVariable<1, VectorField> u("Velocity", "navier_stokes_solution");
  FieldVariable<2, ScalarField> Ct("ThrustCoefficient", "actuator_disk");
  FieldVariable<3, VectorField> uDisk("MeanDiskSpeed", "actuator_disk");
  FieldVariable<4, ScalarField> R("GaussFilter", "actuator_disk");
  const auto pow2 = make_lambda([](Real x ){
	  return x*x;
	    });
  const auto pow3 = make_lambda([](Real x ){
	  return x*x*x;
  });

  boost::mpl::vector<mesh::LagrangeP1::Triag2D, mesh::LagrangeP1::Tetra3D, mesh::LagrangeP1::Quad2D> etypes;

  Handle<ProtoAction> set_filter(get_child("SetFilter"));

  m_prefix = sqrt(6.0) / (sqrt(m_pi) * m_delta);
  set_filter->execute();

  // surface_integral(m_u_mean_disk, std::vector<Handle<mesh::Region>>({m_loop_regions[1]}), _abs((u*normal)[0]));
  // m_u_mean_disk /= m_area;
  m_real_area = 0.0;
  m_u_mean_disk = 0.0;
  volume_integral(m_real_area, std::vector<Handle<mesh::Region>>({m_loop_regions[0]}), R, etypes);
  volume_integral(m_u_mean_disk, std::vector<Handle<mesh::Region>>({m_loop_regions[0]}), u[0]*R, etypes);
  CFinfo << "Real area = " << m_real_area << "   and uR not averaged = " << m_u_mean_disk << CFendl;
  m_u_mean_disk /= (m_real_area);


  m_f = -0.5 * m_ct * m_u_mean_disk * m_u_mean_disk ;//(m_dt * m_u_mean_disk);
  CFinfo << "Mean force to be applied = " << m_f << CFendl;
  // CFinfo << std::setprecision(20) <<"force set to " << m_f << ", a: " << m_a << "m_u_mean_disk :" << m_u_mean_disk <<  " pow2 " << m_u_mean_disk2 << " pow3 " << m_u_mean_disk3 << CFendl;
  options().set("result", m_u_mean_disk);
  Handle<ProtoAction> set_force(get_child("SetForce"));
  Handle<ProtoAction> set_uDisk(get_child("SetuDisk"));
  Handle<ProtoAction> set_ct(get_child("SetCt"));
  
  set_uDisk->execute();
  set_ct->execute();
  set_force->execute();
}


} // namespace adjoint

} // namespace UFEM

} // namespace cf3
