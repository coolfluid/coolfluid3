// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"

#include "Mesh/CList.hpp"

#include "Actions/CLoopOperation.hpp"


/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////
  
void CLoopOperation::define_config_properties( Common::PropertyList& options )
{
}

///////////////////////////////////////////////////////////////////////////////////////

CLoopOperation::CLoopOperation ( const std::string& name ) : 
  CAction(name)
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////////

CList<Uint>& CLoopOperation::loop_list()
{
	throw NotImplemented(FromHere(), "Your operation did not implement the loop_list() function");
}

////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

////////////////////////////////////////////////////////////////////////////////////

