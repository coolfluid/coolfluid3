// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CBuilder.hpp"
#include "common/Foreach.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CCellFaces.hpp"

#include "Solver/Actions/CForAllFaces.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::Mesh;

namespace cf3 {
namespace Solver {
namespace Actions {

ComponentBuilder < CForAllFaces, CAction, LibActions > CForAllFaces_builder;

/////////////////////////////////////////////////////////////////////////////////////

CForAllFaces::CForAllFaces ( const std::string& name ) :
  CLoop(name)
{
}

void CForAllFaces::execute()
{
  boost_foreach(CRegion::Ptr& region, m_loop_regions)
  {
    boost_foreach(CEntities& elements, find_components_recursively_with_tag<CEntities>(*region, Mesh::Tags::face_entity() ) )
    {
      // setup all child operations
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
}

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////
