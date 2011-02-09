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

#include "HeatConductionLinearUnsteady.hpp"

namespace CF {
namespace UFEM {

using namespace Common;
using namespace Common::String;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;
using namespace Solver::Actions::Proto;

CF::Common::ComponentBuilder < HeatConductionLinearUnsteady, LinearSystem, LibUFEM > aHeatConductionLinearUnsteady_Builder;

HeatConductionLinearUnsteady::HeatConductionLinearUnsteady(const std::string& name) : LinearSystemUnsteady(name)
{
}

CFieldAction::Ptr HeatConductionLinearUnsteady::build_equation()
{
  MeshTerm<0, Field<Real> > temperature("Temperature", "T");
  MeshTerm<1, ConfigurableConstant<Real> > alpha("alpha", "Thermal diffusivity (m2/s)");
  MeshTerm<2, ConfigurableConstant<Real> > k("k", "Thermal conductivity (W/(mK))");
  MeshTerm<3, ConstField<Real> > heat("Heat", "q");
  
  return build_elements_action
  (
    "HeatEquation",
    *this,
    group
    (
      system_matrix(lss(), temperature) += integral<1>( (invdt() * sf_outer_product(temperature) + 0.5 * alpha * laplacian(temperature) ) * jacobian_determinant),
      system_rhs(lss(), temperature)    -= alpha * integral<1>( laplacian(temperature) * jacobian_determinant ) * temperature,
      system_rhs(lss(), temperature)    += (alpha / k) * integral<1>( sf_outer_product(temperature) * jacobian_determinant ) * heat
    )
  );
}

} // UFEM
} // CF
