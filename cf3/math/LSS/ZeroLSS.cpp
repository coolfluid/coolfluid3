// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Builder.hpp"

#include "math/LSS/System.hpp"

#include "ZeroLSS.hpp"

namespace cf3 {
namespace math {
namespace LSS {

using namespace common;

common::ComponentBuilder < ZeroLSS, common::Action, LibLSS > ZeroLSS_Builder;

////////////////////////////////////////////////////////////////////////////////

ZeroLSS::ZeroLSS( const std::string& name  ) :
  Action ( name )
{
  mark_basic();

  properties()["brief"] = std::string("Sets  linear system solver back to zero");
  std::string description =
    "This object executes a linear system solver\n";
  properties()["description"] = description;

  options().add("lss", m_lss)
      .description("Linear System solver that gets executed")
      .pretty_name("LSS")
      .mark_basic()
      .link_to(&m_lss);

  options().add("reset_matrix", true)
    .pretty_name("Reset Matrix")
    .description("Should the system matrix be reset to zero?");

  options().add("reset_rhs", true)
    .pretty_name("Reset RHS")
    .description("Should the system Right Hand Side be reset to zero?");

  options().add("reset_solution", true)
    .pretty_name("Reset Solution")
    .description("Should the system solution vector be reset to zero?");
}

////////////////////////////////////////////////////////////////////////////////

void ZeroLSS::execute ()
{
  if(is_null(m_lss))
    throw SetupError(FromHere(), "LSS not set for component " + uri().string());

  bool reset_matrix = options().option("reset_matrix").value<bool>();
  bool reset_rhs = options().option("reset_rhs").value<bool>();
  bool reset_solution = options().option("reset_solution").value<bool>();

  if(reset_matrix)
    m_lss->matrix()->reset();

  if(reset_rhs)
    m_lss->rhs()->reset();

  if(reset_solution)
    m_lss->solution()->reset();
}

////////////////////////////////////////////////////////////////////////////////

} // LSS
} // Math
} // cf3
