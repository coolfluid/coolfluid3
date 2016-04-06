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
#include <common/EventHandler.hpp>

#include "math/LSS/System.hpp"
#include <cmath>
#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "solver/actions/Proto/ElementGradDiv.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"
#include "solver/actions/Proto/Partial.hpp"

#include "BCAdjointke.hpp"
#include "../AdjacentCellToFace.hpp"
#include "../Tags.hpp"

#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{
namespace adjoint
{
using namespace solver::actions::Proto;
using boost::proto::lit;

common::ComponentBuilder < BCAdjointke, common::Action, LibUFEMAdjoint > BCAdjointke_Builder;

BCAdjointke::BCAdjointke(const std::string& name) :
  ProtoAction(name),
  m_dirichlet(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied"))
{
  //options().add("variable_name", "variable_name-NOT_SET")
    //.pretty_name("Variable Name")
    //.description("Name of the variable for which to set the BC")
    //.mark_basic();
    
  //options().add("field_tag", "field_tag-NOT_SET")
    //.pretty_name("Field Tag")
    //.description("Tag of the field to which the variable belongs")
    //.mark_basic();
    
  //options().add("space", "geometry")
    //.pretty_name("Space")
    //.description("Name of the space to use, for example: cf3.mesh.LagrangeP2. Defaults to whatever geometry space is used.")
    //.mark_basic();
    FieldVariable<0, ScalarField> epsilona("epsilona", "Adjointke_solution");
    FieldVariable<1, ScalarField> ka("ka", "Adjointke_solution");
    FieldVariable<2, ScalarField> epsilon("epsilon", "ke_solution");
    FieldVariable<3, ScalarField> k("k", "ke_solution");
  set_expression(nodes_expression(m_dirichlet(epsilona)  = -ka*k/epsilon/m_c_epsilon_1));


}

BCAdjointke::~BCAdjointke()
{
}


} // namespace adjoint
} // namespace UFEM
} // namespace cf3
