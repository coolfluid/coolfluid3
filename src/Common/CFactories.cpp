// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CFactories.hpp"

namespace CF {
namespace Common {

RegistTypeInfo<CFactories> CFactories_TypeInfo();

////////////////////////////////////////////////////////////////////////////////

CFactories::CFactories ( const std::string& name) : Component ( name )
{
  BUILD_COMPONENT;
  TypeInfo::instance().regist<CFactories>(CFactories::type_name());
}

////////////////////////////////////////////////////////////////////////////////

CFactories::~CFactories()
{
}

////////////////////////////////////////////////////////////////////////////////

void CFactories::define_config_properties ( Common::PropertyList& options )
{
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
