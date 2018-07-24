// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/EventHandler.hpp"
#include <cmath>
#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "ActuatorDisk3D.hpp"

#include "solver/actions/Proto/SurfaceIntegration.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{

namespace adjoint
{

using namespace solver::actions::Proto;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ActuatorDisk3D, common::Action, LibUFEMAdjoint > ActuatorDisk3D_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

using boost::proto::lit;

ActuatorDisk3D::ActuatorDisk3D(const std::string& name) :
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
	
  options().add("ct", m_ct)
    .pretty_name("Ct")
    .description("Trust coefficient of the disk")
    .link_to(&m_ct)
    .mark_basic(); // if ct < 0 --> a in function of u_mean_disk

  options().add("yh", m_yh)
    .pretty_name("Hubheight")
    .description("Hub height")
    .link_to(&m_yh)
    .mark_basic(); // is this is enabled, the option can be accessed directly from Python, otherwise .options is needed

  options().add("zh", m_zh)
    .pretty_name("Hubplace")
    .description("Hub place")
    .link_to(&m_zh)
    .mark_basic(); // is this is enabled, the option can be accessed directly from Python, otherwise .options is needed

  options().add("area", m_area)
    .pretty_name("Area")
    .description("Area of the disk")
    .link_to(&m_area)
    .mark_basic(); // is this is enabled, the option can be accessed directly from Python, otherwise .options is needed

  options().add("th", m_th)
    .pretty_name("Thickness")
    .description("Thickness of the disk")
    .link_to(&m_th)
    .mark_basic(); // is this is enabled, the option can be accessed directly from Python, otherwise .options is needed

  // The component that  will set the force
  create_static_component<ProtoAction>("SetForce")->options().option("regions").add_tag("norecurse");

  // Initialize the expression
  trigger_setup();
}

ActuatorDisk3D::~ActuatorDisk3D()
{
}

void ActuatorDisk3D::on_regions_set()
{
  auto regions = options()["regions"].value<std::vector<common::URI>>();

  // Set the regions when the option is set
  get_child("SetForce")->options().set("regions", std::vector<common::URI>({regions[0]}));
}

void ActuatorDisk3D::trigger_setup()
{
    const auto computet=make_lambda([&](const Real y, const Real z)
    {
        const Real R2 = ((y-m_yh)*(y-m_yh))+((z-m_zh)*(z-m_zh));
        const Real lambdar2 = m_omega*m_omega*R2/m_u_mean_disk/m_u_mean_disk*(1-m_a)*(1-m_a);
        const Real ap = -0.5 + (0.5*std::sqrt(1+(4/lambdar2*m_a*(1-m_a))));
        const Real ft = 2*m_u_mean_disk*m_omega*ap*std::sqrt(R2)/m_th;
        const Real angle = std::atan2(y,z);
        RealVector result(3);
		result[0]=m_f;
		if(y>=0){
			if(z>=0){
				result[1]=-std::abs(std::cos(angle)*ft);
                result[2]=std::abs(std::sin(angle)*ft);
				
			}
			else{
		        result[1]=std::abs(std::cos(angle)*ft);
                result[2]=std::abs(std::sin(angle)*ft);
				
			}

		}
		else{
			if(z>=0){
			        result[1]=-std::abs(std::cos(angle)*ft);
                    result[2]=-std::abs(std::sin(angle)*ft);				
				}
				else{
			        result[1]=std::abs(std::cos(angle)*ft);
                    result[2]=-std::abs(std::sin(angle)*ft);					
					
				}

		}

        return result;
    });

  Handle<ProtoAction> set_force(get_child("SetForce"));

  FieldVariable<0, VectorField> f("Force", "body_force");
  FieldVariable<1, VectorField> u("Velocity", "navier_stokes_solution");
  set_force->set_expression(nodes_expression
  (
    group
    (

      f=computet(coordinates[1],coordinates[2])

    )
  ));
}

void ActuatorDisk3D::execute()
{
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  m_u_mean_disk = 0;
  surface_integral(m_u_mean_disk, std::vector<Handle<mesh::Region>>({m_loop_regions[1]}), _abs((u*normal)[0]));
  m_u_mean_disk /= m_area;
  if(m_u_mean_disk > 14*(1-0.0906)){
      m_omega = 12.1/60*2*3.1415;
   }
  else{
      //m_omega = (0.0439*m_u_in+0.6525); function of Uinf
	  m_omega = (0.0487*m_u_mean_disk+0.6471);
  }
  
      if(m_ct<0){
		 m_a = (0.0000000001445*std::pow(m_u_mean_disk, 9))-(0.000000019961*std::pow(m_u_mean_disk, 8))+(0.000001186*std::pow(m_u_mean_disk, 7))
   - (0.000039578*std::pow(m_u_mean_disk, 6))+(0.0008127*std::pow(m_u_mean_disk, 5))-(0.010591*std::pow(m_u_mean_disk, 4))+(0.08739*m_u_mean_disk*m_u_mean_disk*m_u_mean_disk)+(-0.44331*m_u_mean_disk*m_u_mean_disk)+(1.2751*m_u_mean_disk)-1.4627;
        } 
		else{
		m_a = (1-std::sqrt(1-m_ct))/2;	
		}
  
  //m_a = (-0.00000000012263*std::pow(m_u_in, 9))+(0.000000013959*std::pow(m_u_in, 8))+(-0.00000064771*std::pow(m_u_in, 7))
    //      + (0.000015459*std::pow(m_u_in, 6))+(-0.00019067*std::pow(m_u_in, 5))+(0.00084845*std::pow(m_u_in, 4))+(0.0062973*m_u_in*m_u_in*m_u_in)+(-0.099681*m_u_in*m_u_in)+(0.49009*m_u_in)-0.74874;
  m_f = -2 * m_a * m_u_mean_disk *m_u_mean_disk/m_th/(1-m_a);
  CFinfo << "force set to " << m_f << ", a: " << m_a << "m_u_mean_disk :" << m_u_mean_disk << CFendl;

  Handle<ProtoAction> set_force(get_child("SetForce"));
  set_force->execute();
}

} // namespace adjoint

} // namespace UFEM

} // namespace cf3
