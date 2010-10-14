// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CList.hpp"

#include "Actions/CLoopOperation.hpp"
#include "Actions/CForAllNodes.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Actions {

/////////////////////////////////////////////////////////////////////////////////////

void CForAllNodes::defineConfigProperties ( Common::PropertyList& options ) {}

/////////////////////////////////////////////////////////////////////////////////////

CForAllNodes::CForAllNodes ( const CName& name ) :
  CLoop(name)
{
  BUILD_COMPONENT;
}
	
void CForAllNodes::execute()
{
	BOOST_FOREACH(CRegion::Ptr& region, m_loop_regions)
	{
		BOOST_FOREACH(CElements& elements, recursive_range_typed<CElements>(*region))
		{
			// Setup all child operations
			BOOST_FOREACH(CLoopOperation& op, range_typed<CLoopOperation>(*this))
			{
				op.set_loophelper( elements );
				BOOST_FOREACH(const Uint node, op.loop_list().array())
				{
					op.set_loop_idx(node);
					op.execute();
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////