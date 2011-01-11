// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CList.hpp"

#include "Solver/Actions/CNodeOperation.hpp"
#include "Solver/Actions/CForAllNodes.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {
namespace Actions {
  
ComponentBuilder < CForAllNodes, CLoop, LibActions > CForAllNodes_builder;

/////////////////////////////////////////////////////////////////////////////////////

CForAllNodes::CForAllNodes ( const std::string& name ) :
  CLoop(name)
{
}
	
void CForAllNodes::execute()
{
	BOOST_FOREACH(CRegion::Ptr& region, m_loop_regions)
	{
		BOOST_FOREACH(CElements& elements, find_components_recursively<CElements>(*region))
		{
			// Setup all child operations
      CList<Uint>::Ptr loop_list;
      BOOST_FOREACH(CNodeOperation& op, find_components<CNodeOperation>(*this))
			{
        op.create_loop_helper( elements );

        if ( is_null(loop_list) )
          loop_list = op.loop_list().as_type< CList<Uint> >();
        else if (loop_list->size() != op.loop_list().size())
          throw BadValue(FromHere(), "The number of nodes of CNodeOperation [" + op.name() + "] doesn't match with other operations in the same loop");
      }
			BOOST_FOREACH(const Uint node, loop_list->array())
			{
        BOOST_FOREACH(CNodeOperation& op, find_components<CNodeOperation>(*this))
				{
          op.select_loop_idx(node);
					op.execute();
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////
