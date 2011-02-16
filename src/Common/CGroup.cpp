// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @todo can be removed when removing "count" option,
/// as well as the "using namespace boost::assign;" directive.
#include <boost/assign/std/vector.hpp>

#include "Common/CGroup.hpp"
#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"
#include "Common/OptionT.hpp"

using namespace boost::assign; // for operator+=()

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CGroup, Component, LibCommon > CGroup_Builder;

////////////////////////////////////////////////////////////////////////////////

CGroup::CGroup ( const std::string& name ) : Component ( name )
{

}

////////////////////////////////////////////////////////////////////////////////

CGroup::~CGroup()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
