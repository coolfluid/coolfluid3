// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include "common/Log.hpp"
#include "common/OptionT.hpp"
#include "common/CBuilder.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/FindComponents.hpp"
#include "common/Core.hpp"
#include "common/CEnv.hpp"

#include "Math/Consts.hpp"

#include "Solver/Actions/CIterate.hpp"
#include "Solver/Actions/CCriterion.hpp"

namespace cf3 {
namespace Solver {
namespace Actions {

using namespace common;
using namespace Math::Consts;

common::ComponentBuilder < CIterate, CAction, LibActions > CIterate_Builder;

////////////////////////////////////////////////////////////////////////////////

CIterate::CIterate( const std::string& name  ) :
  CAction ( name ),
  m_iter(0),
  m_verbose(false),
  m_max_iter(uint_max())
{
  mark_basic();
  m_properties["brief"] = std::string("Iterator object");
  std::string description =
  "This object handles iterations\n"
  "It can have one or more stop criteria\n";
  m_properties["description"] = description;

  m_options.add_option( OptionT<bool>::create("verbose", m_verbose))
      ->description("Print iteration number")
      ->pretty_name("Verbose")
      ->link_to(&m_verbose);

  m_options.add_option< OptionT<Uint> >("max_iter", m_max_iter)
      ->description("Maximal number of iterations")
      ->pretty_name("Max Iterations")
      ->link_to(&m_max_iter);
}

////////////////////////////////////////////////////////////////////////////////

CIterate::~CIterate()
{
}

////////////////////////////////////////////////////////////////////////////////

void CIterate::execute ()
{
  m_iter=0;
  bool exit_iterations = false;
  while( m_iter != m_max_iter)
  {
    // check if any criterion are met and abort if so
    boost_foreach(CCriterion& stop_criterion, find_components<CCriterion>(*this))
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

    // call all actions and action links inside this component
    boost_foreach(Component& child, children())
    {
      if (CAction::Ptr action = child.follow()->as_ptr<CAction>())
        action->execute();
    }

    // update the iteration
    ++m_iter;
  }
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3
