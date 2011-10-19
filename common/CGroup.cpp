// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CGroup.hpp"
#include "common/CBuilder.hpp"
#include "common/LibCommon.hpp"
#include "common/OptionT.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CGroup, Component, LibCommon > CGroup_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

CGroup::CGroup ( const std::string& name ) : Component ( name )
{

}


CGroup::~CGroup()
{
}

////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
