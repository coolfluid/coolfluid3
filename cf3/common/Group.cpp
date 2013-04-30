// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Group.hpp"
#include "common/Builder.hpp"
#include "common/LibCommon.hpp"
#include "common/OptionT.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Group, Component, LibCommon > Group_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

Group::Group ( const std::string& name ) : Component ( name )
{

}


Group::~Group()
{
}

////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
