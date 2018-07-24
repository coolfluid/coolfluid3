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

#include "DirectDifferentiationSensitivity.hpp"
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

common::ComponentBuilder < DirectDifferentiationSensitivity, common::Action, LibUFEMAdjointTube > DirectDifferentiationSensitivity_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

DirectDifferentiationSensitivity::DirectDifferentiationSensitivity(const std::string& name) :
  Action(name)
{
  properties().add("sensitivity", 0.0);
}

DirectDifferentiationSensitivity::~DirectDifferentiationSensitivity()
{
}

void DirectDifferentiationSensitivity::execute()
{
  if(m_loop_regions.size() != 2)
  {
    throw common::SetupError(FromHere(), "Need exactly 2 regions for DirectDifferentiationSensitivity, the first one is the inlet and the second one the outlet");
  }
  
  std::vector<Handle<mesh::Region>> inlet(1, m_loop_regions[0]);
  std::vector<Handle<mesh::Region>> outlet(1, m_loop_regions[1]);

  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_solution");
  FieldVariable<2, VectorField> SensU("SensU", "sensitivity_solution");
  FieldVariable<3, ScalarField> SensP("SensP", "sensitivity_solution");

  Real inlet_sensitivity = 0.0;
  Real outlet_sensitivity = 0.0;

  surface_integral(inlet_sensitivity, inlet, -(u*normal)*SensP);
  surface_integral(outlet_sensitivity, outlet, SensU*(-(p+0.5*(u*transpose(u))[0])*normal - ((u*normal)[0])*transpose(u)));

  properties().set("sensitivity", inlet_sensitivity + outlet_sensitivity);
}

} // namespace adjointtube

} // namespace UFEM

} // namespace cf3
