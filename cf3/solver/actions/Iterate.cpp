// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Builder.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/FindComponents.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/PropertyList.hpp"

#include "math/Consts.hpp"

#include "solver/actions/Iterate.hpp"
#include "solver/actions/Criterion.hpp"

namespace cf3 {
namespace solver {
namespace actions {

using namespace common;
using namespace math::Consts;

common::ComponentBuilder < Iterate, common::Action, LibActions > Iterate_Builder;

////////////////////////////////////////////////////////////////////////////////

Iterate::Iterate( const std::string& name  ) :
  ActionDirector(name),
  m_iter(0),
  m_verbose(false),
  m_max_iter(uint_max())
{
  mark_basic();
  properties()["brief"] = std::string("Iterator object");
  std::string description =
  "This object handles iterations\n"
  "It can have one or more stop criteria\n";
  properties()["description"] = description;

  options().add_option("verbose", m_verbose)
      .description("Print iteration number")
      .pretty_name("Verbose")
      .link_to(&m_verbose);

  options().add_option("max_iter", m_max_iter)
      .description("Maximal number of iterations")
      .pretty_name("Max Iterations")
      .link_to(&m_max_iter);
}

////////////////////////////////////////////////////////////////////////////////

Iterate::~Iterate()
{
}

////////////////////////////////////////////////////////////////////////////////

void Iterate::execute ()
{
  m_iter=0;
  bool exit_iterations = false;
  while( m_iter != m_max_iter)
  {
    // check if any criterion are met and abort if so
    boost_foreach(Criterion& stop_criterion, find_components<Criterion>(*this))
    {
      if (stop_criterion())
      {
        exit_iterations = true;
        break;
      }
    }
    if (exit_iterations)  break;

    if (m_verbose)
      CFinfo << uri().path() << "[" << m_iter << "]" << CFendl;

    ActionDirector::execute();

    // update the iteration
    ++m_iter;
  }
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
