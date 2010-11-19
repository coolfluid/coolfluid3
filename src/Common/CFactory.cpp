// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CFactory.hpp"

using namespace CF::Common;

CFactory::CFactory(const std::string & lib_name):
    Component(lib_name)
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CFactory::~CFactory()
{
}

////////////////////////////////////////////////////////////////////////////////

void CFactory::define_config_properties ( CF::Common::PropertyList& props )
{
}
