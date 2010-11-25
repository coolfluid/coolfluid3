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

void CForAllNodes::define_config_properties ( Common::PropertyList& options ) {}

/////////////////////////////////////////////////////////////////////////////////////

CForAllNodes::CForAllNodes ( const std::string& name ) :
  CLoop(name)
{
  BuildComponent<full>().build(this);
}
	
void CForAllNodes::execute()
{
	BOOST_FOREACH(CRegion::Ptr& region, m_loop_regions)
	{
		BOOST_FOREACH(CElements& elements, recursive_range_typed<CElements>(*region))
		{
			// Setup all child operations
			CList<Uint>::Ptr loop_list;
			BOOST_FOREACH(CLoopOperation& op, range_typed<CLoopOperation>(*this))
			{
				op.set_loophelper( elements );
				
				if (!loop_list)
					loop_list = op.loop_list().get_type< CList<Uint> >();
				else if (loop_list->size() != op.loop_list().size())
					throw BadValue(FromHere(), "The number of nodes of CLoopOperation [" + op.name() + "] doesn't match with other operations in the same loop");
			}
			BOOST_FOREACH(const Uint node, loop_list->array())
			{
				BOOST_FOREACH(CLoopOperation& op, range_typed<CLoopOperation>(*this))
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