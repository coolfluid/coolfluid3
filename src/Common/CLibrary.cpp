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

CF::RegistTypeInfo<CLibrary> CLibrary_TypeRegistration();

/////////////////////////////////////////////////////////////////////////////////////

CLibrary::CLibrary(const std::string & lib_name) : Component(lib_name),
  m_is_initiated(false)
{
}

////////////////////////////////////////////////////////////////////////////////

CLibrary::~CLibrary()
{
}

std::string CLibrary::lib_kversion()
{
  return CF_KERNEL_VERSION_STR;
}

std::string CLibrary::lib_version()
{
  return CF_KERNEL_VERSION_STR; // by default return the kernel version
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

