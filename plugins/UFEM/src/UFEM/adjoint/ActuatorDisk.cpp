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

#include "ActuatorDisk.hpp"

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

common::ComponentBuilder < ActuatorDisk, common::Action, LibUFEMAdjoint > ActuatorDisk_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

using boost::proto::lit;

ActuatorDisk::ActuatorDisk(const std::string& name) :
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

  options().add("a", m_force_a)
    .pretty_name("Force_a")
    .description("Force a value for a")
    .link_to(&m_force_a)
    .mark_basic();
  
  // The component that  will set the force
  create_static_component<ProtoAction>("SetForce")->options().option("regions").add_tag("norecurse");

  // Initialize the expression
  trigger_setup();
}

ActuatorDisk::~ActuatorDisk()
{
}

void ActuatorDisk::on_regions_set()
{
  auto regions = options()["regions"].value<std::vector<common::URI>>();

  // Set the regions when the option is set
  get_child("SetForce")->options().set("regions", std::vector<common::URI>({regions[0]}));
}

void ActuatorDisk::trigger_setup()
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

void ActuatorDisk::execute()
{
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  m_u_mean_disk = 0;
  m_u_mean_disk2 = 0;
  m_u_mean_disk3 = 0;
  const auto pow2 = make_lambda([](Real x ){
	  return x*x;
	    });
  const auto pow3 = make_lambda([](Real x ){
	  return x*x*x;
  });

  surface_integral(m_u_mean_disk, std::vector<Handle<mesh::Region>>({m_loop_regions[1]}), _abs((u*normal)[0]));
  surface_integral(m_u_mean_disk2, std::vector<Handle<mesh::Region>>({m_loop_regions[1]}), pow2((u*normal/_norm(normal))[0])*_norm(normal));
  surface_integral(m_u_mean_disk3, std::vector<Handle<mesh::Region>>({m_loop_regions[1]}), _abs(pow3((u*normal/_norm(normal))[0])*_norm(normal)));
 

  m_u_mean_disk /= m_area;
  m_u_mean_disk2 /= m_area;
  m_u_mean_disk3 /= m_area;
  if (m_force_a < 0.0)
  {
    if(m_ct<0)
    {
      m_a = (0.0000000001445*std::pow(m_u_mean_disk, 9))-(0.000000019961*std::pow(m_u_mean_disk, 8))+(0.000001186*std::pow(m_u_mean_disk, 7))
    - (0.000039578*std::pow(m_u_mean_disk, 6))+(0.0008127*std::pow(m_u_mean_disk, 5))-(0.010591*std::pow(m_u_mean_disk, 4))+(0.08739*m_u_mean_disk*m_u_mean_disk*m_u_mean_disk)+(-0.44331*m_u_mean_disk*m_u_mean_disk)+(1.2751*m_u_mean_disk)-1.4627;
    } 
    else
    {
      m_a = (1-std::sqrt(1-m_ct))/2;	
    }
  }
  else if (m_force_a >= 0)
  {
    m_a = m_force_a;
  }
  //const Real a = (-0.00000000012263*std::pow(m_u_in, 9))+(0.000000013959*std::pow(m_u_in, 8))+(-0.00000064771*std::pow(m_u_in, 7))
   //+ (0.000015459*std::pow(m_u_in, 6))+(-0.00019067*std::pow(m_u_in, 5))+(0.00084845*std::pow(m_u_in, 4))+(0.0062973*m_u_in*m_u_in*m_u_in)+(-0.099681*m_u_in*m_u_in)+(0.49009*m_u_in)-0.74874;
  m_f = -2 * m_a * m_u_mean_disk*m_u_mean_disk / m_th/(1-m_a);//(m_dt * m_u_mean_disk);
  CFinfo << std::setprecision(20) <<"force set to " << m_f << ", a: " << m_a << "m_u_mean_disk :" << m_u_mean_disk <<  " pow2 " << m_u_mean_disk2 << " pow3 " << m_u_mean_disk3 << CFendl;
  options().set("result", m_u_mean_disk);
  Handle<ProtoAction> set_force(get_child("SetForce"));
  set_force->execute();
}

} // namespace adjoint

} // namespace UFEM

} // namespace cf3
