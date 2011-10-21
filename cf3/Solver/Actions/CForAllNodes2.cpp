// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CBuilder.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "mesh/CRegion.hpp"
#include "mesh/CElements.hpp"
#include "mesh/CList.hpp"

#include "Solver/Actions/CForAllNodes2.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
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
} // cf3

/////////////////////////////////////////////////////////////////////////////////////
