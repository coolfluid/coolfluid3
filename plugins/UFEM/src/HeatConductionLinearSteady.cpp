// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Foreach.hpp"
#include "Common/CBuilder.hpp"

#include "Solver/Actions/Proto/CProtoElementsAction.hpp"
#include "Solver/Actions/Proto/CProtoNodesAction.hpp"

#include "Solver/CEigenLSS.hpp"

#include "HeatConductionLinearSteady.hpp"

namespace CF {
namespace UFEM {

using namespace Common;
using namespace Common::String;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;
using namespace Solver::Actions::Proto;

CF::Common::ComponentBuilder < UFEM::HeatConductionLinearSteady, CMethod, LibUFEM > aHeatConductionLinearSteady_Builder;

HeatConductionLinearSteady::HeatConductionLinearSteady(const std::string& name) : LinearSystem(name)
{
}

CFieldAction::Ptr HeatConductionLinearSteady::build_equation()
{
  MeshTerm<0, ConstField<Real> > temperature("Temperature", "T");
  return build_elements_action
  (
    "HeatEquation",
    *this,
    system_matrix( lss() ) += integral<1>( laplacian(temperature) * jacobian_determinant )
  );
}

} // UFEM
} // CF
