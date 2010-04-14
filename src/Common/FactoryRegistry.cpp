#include "Common/FactoryRegistry.hpp"
#include "Common/FactoryBase.hpp"
#include "Common/Log.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {
  namespace Common {

////////////////////////////////////////////////////////////////////////////////

FactoryRegistry::FactoryRegistry()
{
}

////////////////////////////////////////////////////////////////////////////////

FactoryRegistry::~FactoryRegistry()
{
}

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
    return Common::SafePtr<Common::FactoryBase>(CFNULL);
  }
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////
