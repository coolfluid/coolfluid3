// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionT.hpp"

#include "solver/CTime.hpp"
#include "solver/actions/CCriterionAbsResidual.hpp"

namespace cf3 {
namespace solver {
namespace actions {

using namespace common;

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
      ->description("Maximum number of iterations")
      ->mark_basic()
      ->link_to( &m_max_iter );

  m_options.add_option(OptionComponent<Component>::create("iteration", &m_iter_comp))
      ->description("Iteration tracking component")
      ->pretty_name("Iteration");
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

} // actions
} // solver
} // cf3
