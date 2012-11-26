// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <coolfluid-ufem-config.hpp>

#include "../NavierStokesExplicitAssembly.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <boost/mpl/back_inserter.hpp>
#include <boost/mpl/copy.hpp>

#include "common/Component.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PropertyList.hpp"

#include "solver/actions/Iterate.hpp"
#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/actions/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

using boost::proto::lit;

void NavierStokesExplicit::set_hexa_grad_p_assembly(const solver::actions::Proto::SystemRHS& rhs)
{
#ifdef CF3_UFEM_ENABLE_HEXAS
  set_pressure_gradient_assembly_expression< boost::mpl::vector1<mesh::LagrangeP1::Hexa3D> >("Hexas", rhs);
#endif
}

void NavierStokesExplicit::set_hexa_grad_p_assembly(FieldVariable<3, VectorField>& rhs)
{
#ifdef CF3_UFEM_ENABLE_HEXAS
  set_pressure_gradient_assembly_expression< boost::mpl::vector1<mesh::LagrangeP1::Hexa3D> >("Hexas", rhs);
#endif
}

} // UFEM
} // cf3
