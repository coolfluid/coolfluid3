#include "Common/ModuleRegistry.hpp"
#include "Common/ModuleRegisterBase.hpp"
#include "Common/Log.hpp"
#include "Common/LogLevel.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

void ModuleRegistry::regist(Common::ModuleRegisterBase* module)
{
  if ( !m_store.checkEntry( module->getName()) )
  {
    m_store.addEntry(module->getName(),module);
    CFLogInfo ( "Module " + module->getName() + " registered\n");
  }
  else
  {
    CFLogWarn("Module " + module->getName() + " already registered : skipping registration\n");
  }
}

////////////////////////////////////////////////////////////////////////////////

void ModuleRegistry::unregist(const std::string& moduleName)
{
  if ( m_store.checkEntry( moduleName ) )
  {
    m_store.removeEntry(moduleName);
  }
  else
  {
    CFLogWarn("Module " + moduleName + " not registered : skipping removal\n");
  }
}

////////////////////////////////////////////////////////////////////////////////

bool ModuleRegistry::isRegistered(const std::string& moduleName)
{
  return m_store.checkEntry(moduleName);
}

////////////////////////////////////////////////////////////////////////////////

Common::SafePtr<Common::ModuleRegisterBase>
ModuleRegistry::getModuleRegisterBase(const std::string& moduleName)
{
  if ( m_store.checkEntry( moduleName ) )
  {
    return m_store.getEntry(moduleName);
  }
  else
  {
    CFLogWarn("Module " + moduleName + " not registered : returning null pointer\n");
    return Common::SafePtr<Common::ModuleRegisterBase>(CFNULL);
  }
}

////////////////////////////////////////////////////////////////////////////////

std::vector< Common::SafePtr<Common::ModuleRegisterBase> >
ModuleRegistry::getAllModules()
{
  std::vector< SafePtr<ModuleRegisterBase> > all;
  all.reserve(m_store.size());
  std::transform(m_store.begin(),
                 m_store.end(),
                 back_inserter(all),
                 GeneralStorage<ModuleRegisterBase>::extract);
  return all;
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////
