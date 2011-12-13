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
namespace solver {
namespace actions {

using namespace common;
using namespace math;

common::ComponentBuilder < ZeroLSS, common::Action, LibActions > ZeroLSS_Builder;

////////////////////////////////////////////////////////////////////////////////

ZeroLSS::ZeroLSS( const std::string& name  ) :
  Action ( name )
{
  mark_basic();

  properties()["brief"] = std::string("Sets  linear system solver back to zero");
  std::string description =
    "This object executes a linear system solver\n";
  properties()["description"] = description;

  options().add_option("lss", m_lss)
      .description("Linear System solver that gets executed")
      .pretty_name("LSS")
      .mark_basic()
      .link_to(&m_lss);
}

////////////////////////////////////////////////////////////////////////////////

void ZeroLSS::execute ()
{
  if(is_null(m_lss))
    throw SetupError(FromHere(), "LSS not set for component " + uri().string());

  m_lss->reset();
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
