// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Actions/CDummyLoopOperation.hpp"
#include "Mesh/CFieldElements.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CDummyLoopOperation, CLoopOperation, LibActions > CDummyLoopOperationProvider( "CDummyLoopOperation" );

///////////////////////////////////////////////////////////////////////////////////////

CDummyLoopOperation::CDummyLoopOperation ( const std::string& name ) : 
  CLoopOperation(name)
{
  BuildComponent<none>().build(this);
}
	
/////////////////////////////////////////////////////////////////////////////////////

void CDummyLoopOperation::execute()
{
  CFinfo << "  looping index " << m_idx << CFendl;
}

/////////////////////////////////////////////////////////////////////////////////////

void CDummyLoopOperation::set_loophelper (CElements& geometry_elements )
{
  CFinfo << type_name() << " set to loop over " << geometry_elements.full_path().string() << CFendl;
	data = boost::shared_ptr<LoopHelper> ( new LoopHelper(geometry_elements) );
}

/////////////////////////////////////////////////////////////////////////////////////

CList<Uint>& CDummyLoopOperation::loop_list()
{
	return data->node_list;
}
	
////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

////////////////////////////////////////////////////////////////////////////////////

