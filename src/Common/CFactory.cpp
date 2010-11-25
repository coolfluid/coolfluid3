// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CFactory.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////

CFactory::CFactory(const std::string& name): Component(name)
{
  BuildComponent<full>().build(this);
}

CFactory::~CFactory()
{
}

void CFactory::define_config_properties ( CF::Common::PropertyList& props )
{
}

/////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
