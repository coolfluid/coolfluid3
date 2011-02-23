// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CList.hpp"

#include "Solver/Actions/CForAllNodes2.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {
namespace Actions {
  
ComponentBuilder < CForAllNodes2, CLoop, LibActions > CForAllNodes2_builder;

/////////////////////////////////////////////////////////////////////////////////////

CForAllNodes2::CForAllNodes2 ( const std::string& name ) :
  CLoop(name)
{
}
  
void CForAllNodes2::execute()
{
  boost_foreach(CRegion::Ptr& region, m_loop_regions)
  {
    boost_foreach(CLoopOperation& op, find_components<CLoopOperation>(*this))
    {
     boost_foreach(const Uint node, CElements::used_nodes(*region).array())
      {
        op.select_loop_idx(node);
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
