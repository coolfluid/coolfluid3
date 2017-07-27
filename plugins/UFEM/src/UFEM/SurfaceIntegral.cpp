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
#include "common/XML/SignalOptions.hpp"

#include "math/VariableManager.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Line.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Triag.hpp"

#include "physics/PhysModel.hpp"

#include "SurfaceIntegral.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/SurfaceIntegration.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;

common::ComponentBuilder < SurfaceIntegral, common::Action, LibUFEM > SurfaceIntegral_Builder;

SurfaceIntegral::SurfaceIntegral(const std::string& name) :
  solver::Action(name),
  m_changing_result(false)
{
  options().add("history",m_history)
      .pretty_name("History")
      .description("History component used to log the history of the integral value.")
      .link_to(&m_history)
      .mark_basic();

  options().add("variable_name", "SomeScalar")
    .pretty_name("Variable Name")
    .description("Name of the variable to use")
    .mark_basic();

  options().add("field_tag", "SomeFieldTag")
    .pretty_name("Field Tag")
    .description("Tag for the field to use")
    .mark_basic();

  options().add("result", std::vector<Real>())
      .pretty_name("Result")
      .description("Result of the integration (read-only)")
      .mark_basic();
}

SurfaceIntegral::~SurfaceIntegral()
{
}

void SurfaceIntegral::execute()
{
  const Uint dim = physical_model().ndim();
  m_integral_value.resize(dim);
  m_integral_value.setZero();

  if(m_loop_regions.empty())
  {
    CFwarn << "SurfaceIntegral has no regions" << CFendl;
    return;
  }

  const std::string tag = options().value<std::string>("field_tag");
  const std::string variable_name = options().value<std::string>("variable_name");
  math::VariablesDescriptor& descriptor = common::find_component_with_tag<math::VariablesDescriptor>(physical_model().variable_manager(), tag);
  if(descriptor.dimensionality(variable_name) == math::VariablesDescriptor::Dimensionalities::SCALAR)
  {
    FieldVariable<0, ScalarField> s(variable_name, tag);
    surface_integral(m_integral_value, m_loop_regions, s*normal);
  }
  else
  {
    FieldVariable<0, VectorField> v(variable_name, tag);
    surface_integral(m_integral_value, m_loop_regions, v*normal);
  }

  std::vector<Real> vec_result(dim);
  for(Uint i = 0; i != dim; ++i)
    vec_result[i] = m_integral_value[i];

  m_changing_result = true;
  options().set("result", vec_result);
  m_changing_result = false;

  if(is_not_null(m_history))
  {
    m_history->set(m_variable_name, vec_result);
    m_history->save_entry();
  }
}

void SurfaceIntegral::trigger_result()
{
  if(!m_changing_result)
    throw common::BadValue(FromHere(), "External change of SurfaceIntegral result.");
}

} // namespace UFEM

} // namespace cf3
