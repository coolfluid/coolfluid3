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

#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "TemperatureHistoryHeatConduction.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;
using boost::proto::lit;

common::ComponentBuilder < TemperatureHistoryHeatConduction, common::Action, LibUFEM > TemperatureHistoryHeatConduction_Builder;

TemperatureHistoryHeatConduction::TemperatureHistoryHeatConduction(const std::string& name) :
  ProtoAction(name)
{
    options().option("regions").add_tag("norecurse");
    FieldVariable<0, ScalarField> Thc1("TemperatureHistoryHC1", "temperature_history_hc");
    FieldVariable<1, ScalarField> Thc2("TemperatureHistoryHC2", "temperature_history_hc");

    convergence_history.open ("convergence_history_temperature_hc.dat");

    set_expression(nodes_expression(group(
          lit(m_max_error) = _max(_abs(Thc2 - Thc1), lit(m_max_error))
)));

}

TemperatureHistoryHeatConduction::~TemperatureHistoryHeatConduction()
{
}

void TemperatureHistoryHeatConduction::execute()
{

  m_max_error = 0.;

  ProtoAction::execute();

  convergence_history << m_max_error << "\n";
  if(::fabs(m_max_error) > 1e6)
  {
    convergence_history.close();
    throw common::FailedToConverge(FromHere(), "Temperature exceeded 1 million K");
  }

}

} // namespace UFEM

} // namespace cf3
