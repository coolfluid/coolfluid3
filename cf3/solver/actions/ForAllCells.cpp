// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/Foreach.hpp"

#include "mesh/Region.hpp"
#include "mesh/Cells.hpp"

#include "solver/actions/ForAllCells.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {
namespace actions {

ComponentBuilder < ForAllCells, Loop, LibActions > ForAllCells_builder;

/////////////////////////////////////////////////////////////////////////////////////

ForAllCells::ForAllCells ( const std::string& name ) :
  Loop(name)
{

}

void ForAllCells::execute()
{
  boost_foreach(const Handle<Region>& region, regions())
  {
    boost_foreach(Cells& elements, find_components_recursively<Cells>(*region))
    {
      // Setup all child operations
      boost_foreach(LoopOperation& op, find_components<LoopOperation>(*this))
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
}

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////
