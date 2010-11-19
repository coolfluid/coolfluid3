// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Mesh/CRegion.hpp"

#include "Actions/CForAllElements.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Actions {

/////////////////////////////////////////////////////////////////////////////////////

void CForAllElements::define_config_properties ( Common::PropertyList& options ) {}

/////////////////////////////////////////////////////////////////////////////////////

CForAllElements::CForAllElements ( const CName& name ) :
  CLoop(name)
{
  BUILD_COMPONENT;
}

void CForAllElements::execute()
{
  BOOST_FOREACH(CRegion::Ptr& region, m_loop_regions)
    BOOST_FOREACH(CElements& elements, recursive_range_typed<CElements>(*region))
  {
    // Setup all child operations
    BOOST_FOREACH(CLoopOperation& op, range_typed<CLoopOperation>(*this))
    {
      op.set_loophelper( elements );
      const Uint elem_count = elements.elements_count();
      for ( Uint elem = 0; elem != elem_count; ++elem )
      {
        op.set_loop_idx(elem);
        op.execute();
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////