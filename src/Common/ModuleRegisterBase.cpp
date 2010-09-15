// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

//#include <iostream>

#include "Common/ModuleRegisterBase.hpp"
//#include "Common/CoreEnv.hpp"
#include "Common/ModuleRegistry.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {

////////////////////////////////////////////////////////////////////////////////

ModuleRegisterBase::ModuleRegisterBase(const std::string& name) :
NamedObject(name),
//m_selfRegistry(),
//m_configRegistry(),
m_init(false)
{
//  std::cout << "Registering module [" << name << "]" << std::endl;
//  Common::CoreEnv::instance().getModuleRegistry()->regist(this);
}

////////////////////////////////////////////////////////////////////////////////

ModuleRegisterBase::~ModuleRegisterBase()
{
}

////////////////////////////////////////////////////////////////////////////////

//Common::SelfRegistry& ModuleRegisterBase::getSelfRegistry()
//{
//  return m_selfRegistry;
//}

////////////////////////////////////////////////////////////////////////////////

//Common::ConfigRegistry& ModuleRegisterBase::getConfigRegistry()
//{
//  return m_configRegistry;
//}

////////////////////////////////////////////////////////////////////////////////

void ModuleRegisterBase::initiate()
{
  if (!isInitialized())
  {
    // does nothing by default
    m_init = true;
  }
}

////////////////////////////////////////////////////////////////////////////////

void ModuleRegisterBase::terminate()
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
