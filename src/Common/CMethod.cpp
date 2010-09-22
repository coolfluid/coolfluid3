// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CMethod.hpp"
#include "Common/ObjectProvider.hpp"
#include "Common/LibCommon.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < CMethod, Component, LibCommon, NB_ARGS_1 >
CMethod_Provider ( CMethod::type_name() );

////////////////////////////////////////////////////////////////////////////////

CMethod::CMethod ( const CName& name ) : Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CMethod::~CMethod()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
