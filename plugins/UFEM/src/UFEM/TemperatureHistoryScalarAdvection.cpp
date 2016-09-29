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

#include "TemperatureHistoryScalarAdvection.hpp"
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

common::ComponentBuilder < TemperatureHistoryScalarAdvection, common::Action, LibUFEM > TemperatureHistoryScalarAdvection_Builder;

TemperatureHistoryScalarAdvection::TemperatureHistoryScalarAdvection(const std::string& name) :
  ProtoAction(name)
{
    options().option("regions").add_tag("norecurse");
    FieldVariable<0, ScalarField> Tsa1("TemperatureHistorySA1", "temperature_history_sa");
    FieldVariable<1, ScalarField> Tsa2("TemperatureHistorySA2", "temperature_history_sa");

    convergence_history.open ("convergence_history_temperature_sa.dat");

    set_expression(nodes_expression(group(

          lit(m_max_error) = _max(_abs(Tsa2 - Tsa1), lit(m_max_error))
)));

}

TemperatureHistoryScalarAdvection::~TemperatureHistoryScalarAdvection()
{
}

void TemperatureHistoryScalarAdvection::execute()
{

  m_max_error = 0.;

  ProtoAction::execute();

  convergence_history << m_max_error << "\n";

}

} // namespace UFEM

} // namespace cf3
