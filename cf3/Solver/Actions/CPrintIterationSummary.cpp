// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>
#include <iomanip>

#include "common/PE/Comm.hpp"

#include "Math/Checks.hpp"

#include "common/Log.hpp"
#include "common/CBuilder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"

#include "Solver/Actions/CPrintIterationSummary.hpp"


using namespace cf3::common;
using namespace cf3::Math::Checks;
using namespace cf3::mesh;

namespace cf3 {
namespace Solver {
namespace Actions {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CPrintIterationSummary, CAction, LibActions > CPrintIterationSummary_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

CPrintIterationSummary::CPrintIterationSummary ( const std::string& name ) : CAction(name)
{
  mark_basic();

  // options

  m_options.add_option< OptionT<bool> >("check_convergence", true)
      ->description("checks if the norm contains a non-real number ( either a nan or infinity )");

  m_options.add_option< OptionT<Uint> >("print_rate", 1u)
      ->description("how often to print the iteration summary");

  m_options.add_option(OptionComponent<Component>::create("norm", &my_norm))
      ->description("component holding the norm property");

  m_options.add_option(OptionComponent<Component>::create("iterator", &my_iter))
      ->description("component holding the iteration property");
}


void CPrintIterationSummary::execute()
{
  if( PE::Comm::instance().rank() != 0 ) return;

  // get norm

  if ( my_norm.expired() ) throw SetupError(FromHere(), "Component holding norm was not configured");

  Uint iter = my_iter.lock()->properties().value<Uint>("iteration");
  Real norm = my_norm.lock()->properties().value<Real>("norm");

  Uint print_rate = option("print_rate").value<Uint>();
  bool check_convergence = option("check_convergence").value<bool>();

  if( print_rate > 0 && !(iter % print_rate) )
    CFinfo << "iter ["    << std::setw(4)  << iter << "]"
           << "L2(rhs) [" << std::setw(12) << norm << "]" << CFendl;

  if ( check_convergence && ( is_nan(norm) || is_inf(norm) ) )
    throw FailedToConverge( FromHere(),
                            "Solution diverged after "+to_str(iter)+" iterations");
}

////////////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3
