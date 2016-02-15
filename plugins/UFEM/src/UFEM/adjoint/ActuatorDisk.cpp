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
  options().add("constant", m_constant1)
    .pretty_name("Constant")
    .description("Example constant for use as parameter.")
    .link_to(&m_constant1)
    .mark_basic(); // is this is enabled, the option can be accessed directly from Python, otherwise .options is needed

  options().add("u_mean_in", std::vector<Real>(3,0.))
    .pretty_name("u_mean_in")
    .description("Mean upstream velocity.")
    .attach_trigger(boost::bind(&ActuatorDisk::trigger_u_mean, this))
    .mark_basic();

  // The component that  will set the force
  create_static_component<ProtoAction>("SetForce1")->options().option("regions").add_tag("norecurse");
  create_static_component<ProtoAction>("SetForce2")->options().option("regions").add_tag("norecurse");

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
  if(regions.size() != 2)
  {
    CFwarn << "Need two regions for actuator disk" << CFendl;
    return;
  }
  // Set the regions when the option is set
  get_child("SetForce1")->options().set("regions", std::vector<common::URI>({regions[0]}));
  get_child("SetForce2")->options().set("regions", std::vector<common::URI>({regions[1]}));
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
  Handle<ProtoAction> set_force1(get_child("SetForce1"));
  Handle<ProtoAction> set_force2(get_child("SetForce2"));

  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_solution");
  FieldVariable<2, VectorField> f("Force", "body_force");

  set_force1->set_expression(nodes_expression
  (
    group
    (
       f[0] = -1*lit(m_constant1)*10*10*(1-lit(m_constant1)),
      //lit(u_mean_in) += u,
      //lit(nb_nodes_in) += lit(1.),
      _cout << "added velocity " << nb_nodes_in << " for inlet" << "\n"
    )
  ));

  // Set normal component to zero and tangential component to the wall-law value
  set_force2->set_expression(nodes_expression
  (
    group
    (
      f[0] = -1*lit(m_constant1)*15*15*(1-lit(m_constant)), // First set all components to zero
      //f[0] = lit(m_constant), // Then only the first to a constant
      _cout << "force set to " << nb_nodes_in << "\n"
    )
  ));
}

void ActuatorDisk::execute()
{
  Handle<ProtoAction> set_force1(get_child("SetForce1"));
  Handle<ProtoAction> set_force2(get_child("SetForce2"));

  u_mean_in.setZero();
  nb_nodes_in = 0.;
  set_force1->execute();
  //u_mean_in /= nb_nodes_in;
  set_force2->execute();

  // upstream->execute();
  // u_mean_in /= nb_nodes_in;

}

} // namespace adjoint

} // namespace UFEM

} // namespace cf3
