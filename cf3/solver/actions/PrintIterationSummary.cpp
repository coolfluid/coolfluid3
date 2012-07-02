// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>
#include <iomanip>

#include "common/PE/Comm.hpp"

#include "math/Checks.hpp"

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "solver/actions/PrintIterationSummary.hpp"


using namespace cf3::common;
using namespace cf3::math::Checks;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < PrintIterationSummary, common::Action, LibActions > PrintIterationSummary_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

PrintIterationSummary::PrintIterationSummary ( const std::string& name ) : Action(name)
{
  mark_basic();

  // options

  options().add("check_convergence", false)
      .description("checks if the norm contains a non-real number ( either a nan or infinity )");

  options().add("print_rate", 1u)
      .description("how often to print the iteration summary");

  options().add("norm", my_norm)
      .description("component holding the norm property")
      .link_to(&my_norm);

  options().add("iterator", my_iter)
      .description("component holding the iteration property")
      .link_to(&my_iter);
}


void PrintIterationSummary::execute()
{
  if( PE::Comm::instance().rank() != 0 ) return;

  // get norm

  if ( is_null(my_norm) ) throw SetupError(FromHere(), "Component holding norm was not configured");

  Uint iter = my_iter->properties().value<Uint>("iteration");
  Real norm = my_norm->properties().value<Real>("norm");

  Uint print_rate = options().value<Uint>("print_rate");
  bool check_convergence = options().value<bool>("check_convergence");

  if( print_rate > 0 && !(iter % print_rate) )
    CFinfo << "iter ["    << std::setw(4)  << iter << "]"
           << "L2(rhs) [" << std::setw(12) << norm << "]" << CFendl;

  if ( check_convergence && ( is_nan(norm) || is_inf(norm) ) )
    throw FailedToConverge( FromHere(),
                            "Solution diverged after "+to_str(iter)+" iterations");
}

////////////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
