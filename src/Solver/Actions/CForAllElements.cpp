// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"

#include "Solver/Actions/CForAllElements.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {
namespace Actions {

ComponentBuilder < CForAllElements, CLoop, LibActions > CForAllElements_builder;

/////////////////////////////////////////////////////////////////////////////////////

CForAllElements::CForAllElements ( const std::string& name ) :
  CLoop(name)
{

}

void CForAllElements::execute()
{
  boost_foreach(CRegion::Ptr& region, m_loop_regions)
    boost_foreach(CElements& elements, find_components_recursively<CElements>(*region))
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
