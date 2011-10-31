// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionT.hpp"

#include "solver/CTime.hpp"
#include "solver/actions/CCriterionMaxIterations.hpp"

namespace cf3 {
namespace solver {
namespace actions {

using namespace common;

ComponentBuilder< CCriterionMaxIterations, CCriterion, LibActions > CCriterionMaxIterations_Builder;

////////////////////////////////////////////////////////////////////////////////

CCriterionMaxIterations::CCriterionMaxIterations( const std::string& name  ) :
  CCriterion ( name )
{
  // properties

  m_properties["brief"] = std::string("Maximum Iterations Criterion object");
  std::string description =
      m_properties.value<std::string>("description")+
      "Returns true if a the maximum number of iterations is achived\n";
  m_properties["description"] = description;

  // options

  options().add_option(OptionComponent<Component>::create("iterator", &m_iter_comp))
      ->description("Component performing iterations")
      ->pretty_name("Iterative component");

  options().add_option< OptionT<Uint> >( "maxiter", 1u )
      ->description("Maximum number of iterations (0 will perform none)")
      ->pretty_name("Maximum number");

}

CCriterionMaxIterations::~CCriterionMaxIterations() {}


bool CCriterionMaxIterations::operator()()
{
  if (m_iter_comp.expired())
    throw SetupError(FromHere(),"Component holding iteration number was not set in ["+uri().string()+"]");

  Component& comp_iter = *m_iter_comp.lock();

  const Uint cur_iter = comp_iter.properties().value<Uint>("iteration");
  const Uint max_iter = option("maxiter").value<Uint>();

  return ( cur_iter > max_iter );
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
