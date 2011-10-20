// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CLibrary.hpp"
#include "common/LibCommon.hpp"

namespace cf3 {
namespace common {

RegistTypeInfo<CLibrary,LibCommon> CLibrary_TypeRegistration();

/////////////////////////////////////////////////////////////////////////////////////

CLibrary::CLibrary(const std::string & lib_name) : Component(lib_name),
  m_is_initiated(false)
{
}

////////////////////////////////////////////////////////////////////////////////

CLibrary::~CLibrary()
{
  terminate(); // insure cleanup
}

std::string CLibrary::lib_kversion()
{
  return CF3_KERNEL_VERSION_STR;
}

std::string CLibrary::lib_version()
{
  return CF3_KERNEL_VERSION_STR; // by default return the kernel version
}

void  CLibrary::initiate()
{
  if(!m_is_initiated)
  {
    //CFinfo << "+ initiating library \'" << name() << "\'" << CFendl;
    initiate_impl();
    m_is_initiated = true;
  }
}

void  CLibrary::terminate()
{
  if(m_is_initiated)
  {
    //CFinfo << "+ terminating library \'" << name() << "\'" << CFendl;
    terminate_impl();
    m_is_initiated = false;
  }
}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

