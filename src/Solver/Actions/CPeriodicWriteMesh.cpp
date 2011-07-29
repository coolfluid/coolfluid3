// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CField.hpp"

#include "CPeriodicWriteMesh.hpp"


using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {
namespace Actions {

////////////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CPeriodicWriteMesh, CAction, LibActions > CPeriodicWriteMesh_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

CPeriodicWriteMesh::CPeriodicWriteMesh ( const std::string& name ) : CAction(name)
{
  mark_basic();
}


void CPeriodicWriteMesh::execute()
{

}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF
