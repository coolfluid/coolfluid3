// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "common/List.hpp"

#include "solver/actions/ForAllNodes2.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {
namespace actions {
  
ComponentBuilder < ForAllNodes2, Loop, LibActions > ForAllNodes2_builder;

/////////////////////////////////////////////////////////////////////////////////////

ForAllNodes2::ForAllNodes2 ( const std::string& name ) :
  Loop(name)
{
}
  
void ForAllNodes2::execute()
{
  boost_foreach(Handle< Region >& region, m_loop_regions)
  {
    boost_foreach(LoopOperation& op, find_components<LoopOperation>(*this))
    {
      boost_foreach(const Uint node, Elements::used_nodes(*region).array())
      {
        op.select_loop_idx(node);
        op.execute();
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////
