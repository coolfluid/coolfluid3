// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionComponent.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/MeshMetadata.hpp"
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
  Solver::Action ( name )
{
  mark_basic();

  properties()["brief"] = std::string("Time advancing object");
  std::string description =
    "This object handles time advancing\n";
  properties()["description"] = description;

}

////////////////////////////////////////////////////////////////////////////////

CAdvanceTime::~CAdvanceTime()
{
}

////////////////////////////////////////////////////////////////////////////////

void CAdvanceTime::execute ()
{
  time().time() += time().dt();
  time().iter() += 1u;

  mesh().metadata()["time"] = time().time();
  mesh().metadata()["iter"] = time().iter();

  boost_foreach(CField& field, find_components_recursively<CField>(mesh()))
  {
    field.configure_property("time",time().time());
    field.configure_property("iteration", time().iter());
  }
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF
