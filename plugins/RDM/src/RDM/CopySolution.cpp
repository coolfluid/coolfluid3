// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/Foreach.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/Field.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/IterativeSolver.hpp"

#include "CopySolution.hpp"


using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CopySolution, CAction, LibRDM > CopySolution_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

CopySolution::CopySolution ( const std::string& name ) :
  CF::Solver::Action(name)
{
  mark_basic();

  // options

  options().add_option(
        Common::OptionComponent<Mesh::Field>::create( RDM::Tags::solution(), &m_solution))
      ->pretty_name("Solution");
}

void CopySolution::execute()
{
  RDSolver& mysolver = solver().as_type< RDSolver >();

  if (m_solution.expired())
    m_solution = mysolver.fields().get_child( RDM::Tags::solution() ).follow()->as_ptr_checked<Field>();

  boost_foreach( Component& c, find_components_with_tag( mysolver.fields(), "rksteps" ) )
  {
    Field::Ptr field = c.as_type<CLink>().follow()->as_ptr<Field>();
    if ( field )
    {
      *field = *m_solution.lock();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
