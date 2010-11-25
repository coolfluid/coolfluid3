// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CLibraries.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

CLibraries::CLibraries ( const std::string& name) : Component ( name )
{
  BuildComponent<nosignals>().build(this);
  TypeInfo::instance().regist<CLibraries>(CLibraries::type_name());
}

////////////////////////////////////////////////////////////////////////////////

CLibraries::~CLibraries()
{
}

////////////////////////////////////////////////////////////////////////////////

void CLibraries::define_config_properties ()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
