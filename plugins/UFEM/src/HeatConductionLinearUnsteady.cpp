// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 6
#define BOOST_PROTO_MAX_ARITY 6

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
  MeshTerm< 4, ElementMatrix<0> > A; // Spatial disctitization element matrix
  MeshTerm< 5, ElementMatrix<0> > T; // Temporal disctitization element matrix
  
  return build_elements_action
  (
    "HeatEquation",
    *this,
    group
    (
      A = alpha * integral<1>(laplacian(temperature) * jacobian_determinant),
      T = integral<1>(sf_outer_product(temperature) * jacobian_determinant), // note: we skip multiplying by invdt() so we can reuse this in the source terms
      system_matrix(lss(), temperature) += invdt() * T + 0.5 * A,
      system_rhs(lss(), temperature)    += (alpha / k) * T * heat,
      system_rhs(lss(), temperature)    -= A * temperature
    )
  );
}

} // UFEM
} // CF
