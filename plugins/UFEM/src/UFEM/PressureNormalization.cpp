// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PE/Comm.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/EventHandler.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"
#include "mesh/LagrangeP0/Triag.hpp"
#include "mesh/LagrangeP0/Hexa.hpp"
#include "mesh/LagrangeP0/Tetra.hpp"
#include "mesh/LagrangeP0/Prism.hpp"

#include "PressureNormalization.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/Time.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/VolumeIntegration.hpp"
#include "solver/Tags.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;

common::ComponentBuilder < PressureNormalization, common::Action, LibUFEM > PressureNormalization_Builder;

PressureNormalization::PressureNormalization(const std::string& name) :
  ProtoAction(name)
{
  options().add("pressure_field_tag", "navier_stokes_p_solution")
    .pretty_name("Velocity Field Tag")
    .description("Tag for the field containing the velocity")
    .attach_trigger(boost::bind(&PressureNormalization::trigger_variable, this));

  options().add("pressure_variable_name", "Pressure")
    .pretty_name("Velocity Variable Name")
    .description("Name of the velocity to use")
    .attach_trigger(boost::bind(&PressureNormalization::trigger_variable, this));

  trigger_variable();
}

PressureNormalization::~PressureNormalization()
{
}

void PressureNormalization::trigger_variable()
{
  using boost::proto::lit;

  const std::string tag = options().option("pressure_field_tag").value<std::string>();
  const std::string var = options().option("pressure_variable_name").value<std::string>();

  FieldVariable<0, ScalarField> p(var, tag);
  
  set_expression(nodes_expression
  (
    p -= lit(m_mean)
  ));
}

void PressureNormalization::execute()
{
  using boost::proto::lit;
  const std::string tag = options().option("pressure_field_tag").value<std::string>();
  const std::string var = options().option("pressure_variable_name").value<std::string>();
  FieldVariable<0, ScalarField> p(var, tag);

  volume_integral(m_mean, m_loop_regions, p);
  Real volume = 0.0;
  volume_integral(volume, m_loop_regions, lit(1.0));
  m_mean /= volume;

  ProtoAction::execute();
}



} // namespace UFEM

} // namespace cf3
