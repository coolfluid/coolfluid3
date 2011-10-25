// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/Foreach.hpp"

#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"

#include "solver/actions/CForAllElements.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {
namespace actions {

ComponentBuilder < CForAllElements, CLoop, LibActions > CForAllElements_builder;

/////////////////////////////////////////////////////////////////////////////////////

CForAllElements::CForAllElements ( const std::string& name ) :
  CLoop(name)
{

}

void CForAllElements::execute()
{
  boost_foreach(Region::Ptr& region, m_loop_regions)
    boost_foreach(Elements& elements, find_components_recursively<Elements>(*region))
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

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////
