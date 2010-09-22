// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CGroup.hpp"
#include "Common/ObjectProvider.hpp"
#include "Common/LibCommon.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < CGroup, Component, LibCommon, NB_ARGS_1 >
CGroup_Provider ( CGroup::type_name() );

////////////////////////////////////////////////////////////////////////////////

CGroup::CGroup ( const CName& name ) : Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CGroup::~CGroup()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
