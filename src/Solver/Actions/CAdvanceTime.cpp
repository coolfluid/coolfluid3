// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionComponent.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CField.hpp"
#include "Solver/CModel.hpp"
#include "Solver/CTime.hpp"
#include "Solver/Actions/CAdvanceTime.hpp"

namespace CF {
namespace Solver {
namespace Actions {

using namespace Common;
using namespace Mesh;

Common::ComponentBuilder < CAdvanceTime, CAction, LibActions > CAdvanceTime_Builder;

////////////////////////////////////////////////////////////////////////////////

CAdvanceTime::CAdvanceTime( const std::string& name  ) :
  CAction ( name )
{
  mark_basic();

  properties()["brief"] = std::string("Time advancing object");
  std::string description =
    "This object handles time advancing\n";
  properties()["description"] = description;

  properties().add_option(OptionComponent<CTime>::create("Time","Time tracking component",&m_time))
    ->mark_basic()
    ->add_tag("time");

}

////////////////////////////////////////////////////////////////////////////////

CAdvanceTime::~CAdvanceTime()
{
}

////////////////////////////////////////////////////////////////////////////////

void CAdvanceTime::execute ()
{
  if (m_time.expired())
    throw SetupError(FromHere(),"Time option was not set in ["+full_path().path()+"]");
  m_time.lock()->time() += m_time.lock()->dt();

  CModel& model = find_parent_component<CModel>(*this);
  boost_foreach(CField& field, find_components_recursively<CField>(model))
  {
    field.configure_property("time",m_time.lock()->time());
    field.configure_property("iteration", field.property("iteration").value<Uint>()+1);
  }
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF
