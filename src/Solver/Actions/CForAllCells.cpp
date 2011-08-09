// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CCells.hpp"

#include "Solver/Actions/CForAllCells.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {
namespace Actions {

ComponentBuilder < CForAllCells, CLoop, LibActions > CForAllCells_builder;

/////////////////////////////////////////////////////////////////////////////////////

CForAllCells::CForAllCells ( const std::string& name ) :
  CLoop(name)
{

}

void CForAllCells::execute()
{
  boost_foreach(CRegion& region, regions())
    boost_foreach(CCells& elements, find_components_recursively<CCells>(region))
  {
    // Setup all child operations
    boost_foreach(CLoopOperation& op, find_components<CLoopOperation>(*this))
    {
      op.set_elements(elements);
      if (op.can_start_loop())
      {
        const Uint nb_elem = elements.size();
        for ( Uint elem = 0; elem != nb_elem; ++elem )
        {
          op.select_loop_idx(elem);
          op.execute();
        }
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////
