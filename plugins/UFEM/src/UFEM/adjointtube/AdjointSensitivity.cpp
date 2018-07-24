// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <string>
#include "common/EigenAssertions.hpp"
#include <Eigen/Dense>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include <common/EventHandler.hpp>

#include <vector>
#include <iostream>
// #include "math/LSS/System.hpp"
#include <cmath>


#include "mesh/Region.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "AdjointSensitivity.hpp"
#include "../Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/SurfaceIntegration.hpp"

namespace cf3
{

namespace UFEM
{

namespace adjointtube
{

using namespace solver::actions::Proto;
using namespace std;
using namespace Eigen;
using boost::proto::lit;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < AdjointSensitivity, common::Action, LibUFEMAdjointTube > AdjointSensitivity_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

AdjointSensitivity::AdjointSensitivity(const std::string& name) :
  Action(name)
{
  properties().add("sensitivity", 0.0);
}

AdjointSensitivity::~AdjointSensitivity()
{
}

void AdjointSensitivity::execute()
{
  FieldVariable<0, VectorField> n("NodalNormal", "nodal_normals");
  FieldVariable<1, ScalarField> q("AdjPressure", "adjoint_solution");
  FieldVariable<2, VectorField> grad_Ux("grad_Ux", "Adjvelocity_gradient");
  FieldVariable<3, VectorField> grad_ux("grad_ux", "velocity_gradient");
  FieldVariable<4, ScalarField> J("SensDer", "sensitivity_derivative");//, mesh::LagrangeP0::LibLagrangeP0::library_namespace());
  FieldVariable<5, VectorField> grad_Uy("grad_Uy", "Adjvelocity_gradient");
  FieldVariable<6, VectorField> grad_uy("grad_uy", "velocity_gradient");
  FieldVariable<7, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");
  FieldVariable<4, ScalarField> node_filter("node_filter", "node_filter");

  Real sensitivity = 0.0;

  surface_integral(sensitivity, m_loop_regions, (-(nu_eff*(grad_Ux*normal)[0] - q*normal[0])*grad_ux - (nu_eff*(grad_Uy*normal)[0] - q*normal[1])*grad_uy) * transpose(n) * node_filter);

  properties().set("sensitivity", sensitivity);
}

void AdjointSensitivity::on_physical_model_changed()
{
  std::cout << "adding node_filter descriptor" << std::endl;
  auto descriptor = physical_model().variable_manager().create_component<math::VariablesDescriptor>("node_filter");
  descriptor->add_tag("node_filter");
  descriptor->push_back("node_filter", math::VariablesDescriptor::Dimensionalities::SCALAR);
}

} // namespace adjointtube

} // namespace UFEM

} // namespace cf3
