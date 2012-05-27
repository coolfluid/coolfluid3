// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/PropertyList.hpp"

#include "solver/Time.hpp"
#include "solver/actions/CriterionMaxIterations.hpp"

namespace cf3 {
namespace solver {
namespace actions {

using namespace common;

ComponentBuilder< CriterionMaxIterations, Criterion, LibActions > CriterionMaxIterations_Builder;

////////////////////////////////////////////////////////////////////////////////

CriterionMaxIterations::CriterionMaxIterations( const std::string& name  ) :
  Criterion ( name )
{
  // properties

  properties()["brief"] = std::string("Maximum Iterations Criterion object");
  std::string description =
      properties().value<std::string>("description")+
      "Returns true if a the maximum number of iterations is achived\n";
  properties()["description"] = description;

  // options

  options().add("iterator", m_iter_comp)
      .description("Component performing iterations")
      .pretty_name("Iterative component")
      .link_to(&m_iter_comp);

  options().add( "maxiter", 1u )
      .description("Maximum number of iterations (0 will perform none)")
      .pretty_name("Maximum number");

}

CriterionMaxIterations::~CriterionMaxIterations() {}


bool CriterionMaxIterations::operator()()
{
  if (is_null(m_iter_comp))
    throw SetupError(FromHere(),"Component holding iteration number was not set in ["+uri().string()+"]");

  Component& comp_iter = *m_iter_comp;

  const Uint cur_iter = comp_iter.properties().value<Uint>("iteration");
  const Uint max_iter = options().value<Uint>("maxiter");

  return ( cur_iter > max_iter );
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
