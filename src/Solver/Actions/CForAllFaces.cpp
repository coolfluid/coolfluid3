// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CRegion.hpp"

#include "Solver/Actions/CForAllFaces.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {
namespace Actions {

ComponentBuilder < CForAllFaces, CLoop, LibActions > CForAllFaces_builder;

/////////////////////////////////////////////////////////////////////////////////////

CForAllFaces::CForAllFaces ( const std::string& name ) :
  CLoop(name)
{
   
}

void CForAllFaces::execute()
{
  boost_foreach(CRegion::Ptr& region, m_loop_regions)
    boost_foreach(CElements& elements, find_components_recursively_with_filter<CElements>(*region,IsElementsSurface()))
  {
    // Setup all child operations
    boost_foreach(CLoopOperation& op, find_components<CLoopOperation>(*this))
    {
      op.create_loop_helper( elements );
      const Uint elem_count = elements.elements_count();
      for ( Uint elem = 0; elem != elem_count; ++elem )
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
