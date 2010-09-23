// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ProviderBase.hpp"

#include "Common/Log.hpp"
#include "Common/LogLevel.hpp"

#include "Common/SelfRegistry.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {

//////////////////////////////////////////////////////////////////////////////

SelfRegistry::SelfRegistry()
{
}

//////////////////////////////////////////////////////////////////////////////

SelfRegistry::~SelfRegistry()
{
}

//////////////////////////////////////////////////////////////////////////////

void SelfRegistry::regist(ProviderBase* provider)
{
  const std::string& name = provider->getProviderName();
  const std::string& type = provider->getProviderType();
  if ( !m_store[type].checkEntry(name) )
  {
    m_store[type].addEntry(name,provider);
  }
  else
  {
    CFwarn << "Provider [" << provider->getProviderName()
        << "] of type [" << provider->getProviderType()
        << "] already registered : skipping registration\n";
  }
}

//////////////////////////////////////////////////////////////////////////////

void SelfRegistry::unregist(const std::string& name, const std::string& type)
{
  if ( m_store[type].checkEntry(name) )
  {
    m_store[type].removeEntry(name);
  }
  else
  {
    CFwarn << "Provider [" << name << "] of type [" << type
        << "] not registered : skipping removal\n";
  }
}

//////////////////////////////////////////////////////////////////////////////

void SelfRegistry::unregist(ProviderBase* provider)
{

  const std::string& name = provider->getProviderName();
  const std::string& type = provider->getProviderType();
  if ( m_store[type].checkEntry(name) )
  {
    unregist(name,type);
  }
  else
  {
    CFwarn << "Provider ["  << provider->getProviderName()
        << "] of type [" << provider->getProviderType()
        << "] not registered : skipping removal\n";
  }
}

//////////////////////////////////////////////////////////////////////////////

SafePtr<ProviderBase>
SelfRegistry::getProvider(const std::string& name, const std::string& type)
{
  if ( m_store[type].checkEntry(name) )
  {
    return m_store[type].getEntry(name);
  }
  else
  {
    CFwarn << "Provider [" << name << "] of type [" << type
        << "] not registered : returning null pointer\n";

    return SafePtr<ProviderBase>(CFNULL);
  }
}

//////////////////////////////////////////////////////////////////////////////

  } // namespace Common
} // namespace CF

//////////////////////////////////////////////////////////////////////////////
