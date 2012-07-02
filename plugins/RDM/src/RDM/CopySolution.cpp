// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Field.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/IterativeSolver.hpp"

#include "CopySolution.hpp"


using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CopySolution, common::Action, LibRDM > CopySolution_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

CopySolution::CopySolution ( const std::string& name ) :
  cf3::solver::Action(name)
{
  mark_basic();

  // options

  options().add(RDM::Tags::solution(), m_solution)
      .pretty_name("Solution")
      .link_to(&m_solution);
}

void CopySolution::execute()
{
  RDSolver& mysolver = *solver().handle< RDSolver >();

  if (is_null(m_solution))
    m_solution = follow_link(mysolver.fields().get_child( RDM::Tags::solution() ))->handle<Field>();

  boost_foreach( Component& c, find_components_with_tag( mysolver.fields(), "past_step" ) )
  {
    Handle< Field > field(follow_link(c));
    if ( field )
    {
      *field = *m_solution;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
