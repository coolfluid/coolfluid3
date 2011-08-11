// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CGroup.hpp"
#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"
#include "Common/OptionT.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CGroup, Component, LibCommon > CGroup_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

CGroup::CGroup ( const std::string& name ) : Component ( name )
{

}


CGroup::~CGroup()
{
}

////////////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
