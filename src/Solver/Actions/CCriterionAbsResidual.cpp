// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"

#include "Solver/CTime.hpp"
#include "Solver/Actions/CCriterionAbsResidual.hpp"

namespace CF {
namespace Solver {
namespace Actions {

using namespace Common;

ComponentBuilder< CCriterionAbsResidual, CCriterion, LibActions > CCriterionAbsResidual_Builder;

////////////////////////////////////////////////////////////////////////////////

CCriterionAbsResidual::CCriterionAbsResidual( const std::string& name  ) :
  CCriterion ( name ),
  m_max_iter(0)
{
  m_properties["brief"] = std::string("Maximum Iterations Criterion object");
  std::string description = properties().value<std::string>("description")+
    "Returns true if a the maximum number of iterations is achived\n";
  m_properties["description"] = description;

  m_options.add_option<OptionT <Uint> >("MaxIter", m_max_iter)
      ->set_description("Maximum number of iterations")
      ->mark_basic()
      ->link_to( &m_max_iter );

  m_options.add_option(OptionComponent<Component>::create("iteration", &m_iter_comp))
      ->set_description("Iteration tracking component")
      ->set_pretty_name("Iteration");
}

CCriterionAbsResidual::~CCriterionAbsResidual() {}

////////////////////////////////////////////////////////////////////////////////

bool CCriterionAbsResidual::operator()()
{
  if (m_iter_comp.expired()) throw SetupError(FromHere(),"Component holding iteration number was not set in ["+uri().string()+"]");
  Component& comp_iter = *m_iter_comp.lock();

  const Uint cur_iter = comp_iter.option("iter").value<Uint>();

  return ( cur_iter > m_max_iter );
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF
