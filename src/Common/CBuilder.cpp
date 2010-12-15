// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

namespace CF {
namespace Common {

RegistTypeInfo<CBuilder> CBuilder_TypeRegistration();

////////////////////////////////////////////////////////////////////////////////

CBuilder::CBuilder ( const std::string& name) : Component ( name )
{
}

////////////////////////////////////////////////////////////////////////////////

CBuilder::~CBuilder()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
