// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>

#include "Teuchos_ParameterList.hpp"

#include "common/Builder.hpp"

#include "BelosGMRESParameters.hpp"

namespace cf3 {
namespace math {
namespace LSS {

common::ComponentBuilder<BelosGMRESParameters, ParameterListDefaults, LibLSS> BelosGMRESParameters_builder;

BelosGMRESParameters::BelosGMRESParameters(const std::string& name) : ParameterListDefaults(name)
{
}

void BelosGMRESParameters::set_parameters(Teuchos::ParameterList& parameters) const
{
  parameters.set("Linear Solver Type", "Belos");
  Teuchos::ParameterList& belos = parameters.sublist("Linear Solver Types").sublist("Belos");
  belos.set("Solver Type", "Block GMRES");
  Teuchos::ParameterList& gmres = belos.sublist("Solver Types").sublist("Block GMRES");
  gmres.set("Maximum Iterations", 200);
  gmres.set("Convergence Tolerance", 1e-10);
  gmres.set("Num Blocks", 20);
  gmres.set("Verbosity", 0);

  parameters.set("Preconditioner Type", "Ifpack");
  parameters.sublist("Preconditioner Types").sublist("Ifpack").set("Prec Type", "ILU");
}



} // namespace LSS
} // namespace math
} // namespace cf3
