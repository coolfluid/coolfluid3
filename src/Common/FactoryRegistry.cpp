// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/FactoryRegistry.hpp"
#include "Common/Factory.hpp"
#include "Common/Log.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

void FactoryRegistry::regist(Common::FactoryBase* factory)
{
  const std::string type_name = factory->getTypeName();
  if ( ! m_store.checkEntry(type_name) )
  {
    m_store.addEntry(type_name, factory);
//     CFLogInfo (  "Factory [" + type_name + "] registered\n" );
  }
  else
  {
    CFLogWarn("Factory " + factory->getTypeName() + " already registered : skipping registration\n");
  }
}

////////////////////////////////////////////////////////////////////////////////

void FactoryRegistry::unregist(const std::string& type_name)
{
  if ( m_store.checkEntry(type_name) )
  {
    m_store.removeEntry(type_name);
//     CFLogInfo (  "Factory [" + type_name + "] unregistered\n" );
  }
  else
  {
    CFLogWarn("Factory [" + type_name + "] not registered : skipping removal\n");
  }
}

////////////////////////////////////////////////////////////////////////////////

Common::SafePtr<Common::FactoryBase>
FactoryRegistry::getFactory(const std::string& type_name)
{
  if ( m_store.checkEntry(type_name) )
  {
//     CFLogInfo (  "Factory [" + type_name + "] found and returning\n" );
    return m_store.getEntry(type_name);
  }
  else
  {
    CFLogWarn("Factory [" + type_name + "] not registered : returning null pointer\n");
    return Common::SafePtr<Common::FactoryBase>(nullptr);
  }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////
