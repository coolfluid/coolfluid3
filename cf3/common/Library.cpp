// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Library.hpp"
#include "common/LibCommon.hpp"

namespace cf3 {
namespace common {

RegistTypeInfo<Library,LibCommon> Library_TypeRegistration();

/////////////////////////////////////////////////////////////////////////////////////

Library::Library(const std::string & lib_name) : Component(lib_name),
  m_is_initiated(false)
{
}

////////////////////////////////////////////////////////////////////////////////

Library::~Library()
{
  // No terminate allowed here. See http://www.artima.com/cppsource/pure_virtual.html
}

std::string Library::lib_kversion()
{
  return CF3_KERNEL_VERSION_STR;
}

std::string Library::lib_version()
{
  return CF3_KERNEL_VERSION_STR; // by default return the kernel version
}

void  Library::initiate()
{
  if(!m_is_initiated)
  {
    //CFinfo << "+ initiating library \'" << name() << "\'" << CFendl;
    initiate_impl();
    m_is_initiated = true;
  }
}

void  Library::terminate()
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

