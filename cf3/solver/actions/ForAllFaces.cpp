// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/Foreach.hpp"

#include "mesh/Region.hpp"

#include "solver/actions/ForAllFaces.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {
namespace actions {

ComponentBuilder < ForAllFaces, common::Action, LibActions > ForAllFaces_builder;

/////////////////////////////////////////////////////////////////////////////////////

ForAllFaces::ForAllFaces ( const std::string& name ) :
  Loop(name)
{
}

void ForAllFaces::execute()
{
  boost_foreach(Handle< Region >& region, m_loop_regions)
  {
    boost_foreach(Entities& elements, find_components_recursively_with_tag<Entities>(*region, mesh::Tags::face_entity() ) )
    {
      // setup all child operations
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
