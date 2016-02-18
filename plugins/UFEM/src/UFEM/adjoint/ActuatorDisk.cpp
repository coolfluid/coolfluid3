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

#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "ActuatorDisk.hpp"

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

common::ComponentBuilder < ActuatorDisk, common::Action, LibUFEMAdjoint > ActuatorDisk_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

using boost::proto::lit;

ActuatorDisk::ActuatorDisk(const std::string& name) :
  Action(name),
  rhs(options().add("lss", Handle<math::LSS::System>())
    .pretty_name("LSS")
    .description("The linear system for which the boundary condition is applied")),
  system_matrix(options().option("lss")),
  u_mean_in(3)
{
  options().add("Uin", U_in)
    .pretty_name("Velocityin")
    .description("Inlet velocity")
    .link_to(&U_in)
    .mark_basic(); // is this is enabled, the option can be accessed directly from Python, otherwise .options is needed

  options().add("Area", A)
    .pretty_name("Area")
    .description("Area of the disk")
    .link_to(&A)
    .mark_basic(); // is this is enabled, the option can be accessed directly from Python, otherwise .options is needed

  options().add("Timestep", Timestep)
    .pretty_name("Timestep")
    .description("Timestep")
    .link_to(&Timestep)
    .mark_basic(); // is this is enabled, the option can be accessed directly from Python, otherwise .options is needed

  options().add("u_mean_in", std::vector<Real>(3,0.))
    .pretty_name("u_mean_in")
    .description("Mean upstream velocity.")
    .attach_trigger(boost::bind(&ActuatorDisk::trigger_u_mean, this))
    .mark_basic();

  // The component that  will set the force
  create_static_component<ProtoAction>("SetForce")->options().option("regions").add_tag("norecurse");


  // Initialize the expression
  trigger_setup();
  // Initialize mean velocity
  trigger_u_mean();
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

void ActuatorDisk::trigger_u_mean()
{
  auto u_mean_vec = options().value<std::vector<Real>>("u_mean_in");
  if(u_mean_vec.size() != 3)
  {
    throw common::SetupError(FromHere(), "Expected 3 components for u_mean_in");
  }

  u_mean_in = Eigen::Map<RealVector>(&u_mean_vec[0], 3);
}

void ActuatorDisk::trigger_setup()
{
  Handle<ProtoAction> set_force(get_child("SetForce"));
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_solution");
  FieldVariable<2, VectorField> f("Force", "body_force");


  set_force->set_expression(nodes_expression
  (
    group
    (
        CT = ((-0.0000000000011324*U_in*U_in*U_in*U_in*U_in*U_in*U_in*U_in*U_in)+(0.00000000015357*U_in*U_in*U_in*U_in*U_in*U_in*U_in*U_in)+(-0.000000009002*U_in*U_in*U_in*U_in*U_in*U_in*U_in)+(0.00000029882*U_in*U_in*U_in*U_in*U_in*U_in)+(-0.0000061814*U_in*U_in*U_in*U_in*U_in)+(0.000082595*U_in*U_in*U_in*U_in)+(-0.00071366*U_in*U_in*U_in)+(0.0038637*U_in*U_in)+(-0.012101*U_in)+0.017983),
        f[0] = -0.5*CT*U_in*U_in*A/Timestep/u_mean_in[0],

      _cout << "force set to " << transpose(f) << "CT" << CT << "\n"
    )
  ));
}

void ActuatorDisk::execute()
{
  Handle<ProtoAction> set_force(get_child("SetForce"));
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  CT = 0.;
  u_mean_in.setZero();
  surface_integral(u_mean_in, m_loop_regions, u*normal);
  set_force->execute();


}

} // namespace adjoint

} // namespace UFEM

} // namespace cf3
