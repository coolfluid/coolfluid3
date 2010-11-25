// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/LibCommon.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CLibrary.hpp"

namespace CF {
namespace Common {

CF::RegistTypeInfo<CLibrary> sCLibrary_regist();

/////////////////////////////////////////////////////////////////////////////////////

CLibrary::CLibrary(const std::string & lib_name):
    Component(lib_name)
{
  BuildComponent<full>().build(this);
}

////////////////////////////////////////////////////////////////////////////////

CLibrary::~CLibrary()
{

}

////////////////////////////////////////////////////////////////////////////////

void CLibrary::define_config_properties ( CF::Common::PropertyList& props )
{
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

