// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CRegion.hpp"

#include "Solver/Actions/CForAllElements2.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {
namespace Actions {

ComponentBuilder < CForAllElements2, CLoop, LibActions > CForAllElements2_builder;

/////////////////////////////////////////////////////////////////////////////////////

CForAllElements2::CForAllElements2 ( const std::string& name ) :
  CLoop(name)
{
   
}

void CForAllElements2::execute()
{
  boost_foreach(CRegion::Ptr& region, m_loop_regions)
    boost_foreach(CElements& elements, find_components_recursively<CElements>(*region))
  {
    // Setup all child operations
    boost_foreach(CLoopOperation& op, find_components<CLoopOperation>(*this))
    {
      op.configure_property("Elements",elements.full_path());
      const Uint nb_elem = elements.size();
      for ( Uint elem = 0; elem != nb_elem; ++elem )
      {
        op.select_loop_idx(elem);
        op.execute();
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////
