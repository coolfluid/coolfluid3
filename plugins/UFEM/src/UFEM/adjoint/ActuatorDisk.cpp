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
  options().add("constant", m_constant)
    .pretty_name("Constant")
    .description("Example constant for use as parameter.")
    .link_to(&m_constant)
    .mark_basic(); // is this is enabled, the option can be accessed directly from Python, otherwise .options is needed

  // The component that  will set the force
  create_static_component<ProtoAction>("SetForce")->options().option("regions").add_tag("norecurse");
  create_static_component<ProtoAction>("Upstream")->options().option("regions").add_tag("norecurse");

  // Initialize the expression
  trigger_setup();
}

ActuatorDisk::~ActuatorDisk()
{
}


void ActuatorDisk::on_regions_set()
{
  auto regions = options()["regions"].value<std::vector<common::URI>>();
  if(regions.size() != 2)
  {
    CFwarn << "Need two regions for actuator disk" << CFendl;
    return;
  }
  // Set the regions when the option is set
  get_child("SetForce")->options().set("regions", std::vector<common::URI>({regions[0]}));
  get_child("Upstream")->options().set("regions", std::vector<common::URI>({regions[1]}));
}

void ActuatorDisk::trigger_setup()
{
  Handle<ProtoAction> set_force(get_child("SetForce"));
  Handle<ProtoAction> upstream(get_child("Upstream"));

  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_solution");
  FieldVariable<2, VectorField> f("Force", "body_force");

  upstream->set_expression(nodes_expression
  (
    group
    (
      lit(u_mean_in) += u,
      lit(nb_nodes_in) += lit(1.),
      _cout << "added velocity " << transpose(u) << " for inlet" << "\n"
    )
  ));

  // Set normal component to zero and tangential component to the wall-law value
  set_force->set_expression(nodes_expression
  (
    group
    (
      f[0] = -0.5*lit(m_constant)*lit(u_mean_in[0])*lit(u_mean_in[0]), // First set all components to zero
      //f[0] = lit(m_constant), // Then only the first to a constant
      _cout << "force set to " << transpose(f) << "\n"
    )
  ));
}

void ActuatorDisk::execute()
{
  Handle<ProtoAction> set_force(get_child("SetForce"));
  Handle<ProtoAction> upstream(get_child("Upstream"));

  u_mean_in.setZero();
  nb_nodes_in = 0.;
  upstream->execute();
  u_mean_in /= nb_nodes_in;
  set_force->execute();
}

} // namespace adjoint

} // namespace UFEM

} // namespace cf3
