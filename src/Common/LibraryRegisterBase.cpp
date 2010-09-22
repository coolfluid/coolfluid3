// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

//#include <iostream>

//#include "Common/CoreEnv.hpp"
#include "Common/LibraryRegistry.hpp"

#include "Common/LibraryRegisterBase.hpp"


////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {

////////////////////////////////////////////////////////////////////////////////

LibraryRegisterBase::LibraryRegisterBase(const std::string& name) :
NamedObject(name),
//m_selfRegistry(),
//m_configRegistry(),
m_init(false)
{
//  std::cout << "Registering module [" << name << "]" << std::endl;
//  Common::CoreEnv::instance().getLibraryRegistry()->regist(this);
}

////////////////////////////////////////////////////////////////////////////////

LibraryRegisterBase::~LibraryRegisterBase()
{
}

////////////////////////////////////////////////////////////////////////////////

//Common::SelfRegistry& LibraryRegisterBase::getSelfRegistry()
//{
//  return m_selfRegistry;
//}

////////////////////////////////////////////////////////////////////////////////

//Common::ConfigRegistry& LibraryRegisterBase::getConfigRegistry()
//{
//  return m_configRegistry;
//}

////////////////////////////////////////////////////////////////////////////////

void LibraryRegisterBase::initiate()
{
  if (!isInitialized())
  {
    // does nothing by default
    m_init = true;
  }
}

////////////////////////////////////////////////////////////////////////////////

void LibraryRegisterBase::terminate()
{
  if(isInitialized())
  {
    // does nothing by default
    m_init = false;
  }
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////
