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
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;
using namespace Solver::Actions::Proto;

CF::Common::ComponentBuilder < UFEM::HeatConductionLinearSteady, LinearSystem, LibUFEM > aHeatConductionLinearSteady_Builder;

HeatConductionLinearSteady::HeatConductionLinearSteady(const std::string& name) : LinearSystem(name)
{
}

CFieldAction::Ptr HeatConductionLinearSteady::build_equation()
{
  MeshTerm<0, Field<Real> > temperature("Temperature", "T");
  MeshTerm<1, ConfigurableConstant<Real> > k("k", "Thermal conductivity (J/(mK))");
  MeshTerm<2, ConstField<Real> > heat("Heat", "q");

  return build_elements_action
  (
    "HeatEquation",
    *this,
    group
    (
      system_matrix( lss(), temperature ) +=  k * integral<1>( laplacian(temperature) * jacobian_determinant ),
      system_rhs( lss(), temperature ) += integral<1>( sf_outer_product(temperature) * jacobian_determinant ) * heat
    )
  );
}

} // UFEM
} // CF
