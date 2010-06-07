#ifndef CF_Common_ModuleRegistry_hpp
#define CF_Common_ModuleRegistry_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/SafePtr.hpp"
#include "Common/GeneralStorage.hpp"

#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

    class ModuleRegisterBase;

////////////////////////////////////////////////////////////////////////////////

/// This class is a singleton object which serves as registry for all the
/// modules that are loaded
/// The only instance of this object is held by the CFEnv.
/// @author Tiago Quintino
class Common_API ModuleRegistry : public boost::noncopyable {

public:

  /// Register a module
  /// @param module pointer to a ModuleRegisterBase to be added
  void regist(Common::ModuleRegisterBase* module);

  /// Remove a registered module
  /// @param moduleName name of a ModuleRegisterBase to be removed
  void unregist(const std::string& moduleName);

  /// Checks that a module is registered
  /// @param moduleName name of a ModuleRegisterBase to be checked
  bool isRegistered(const std::string& moduleName);

  /// Get a given module
  /// @param moduleName name of a ModuleRegisterBase to be accessed
  /// @return a pointer to a ModuleRegisterBase if found or a null pointer if not found
  Common::SafePtr<Common::ModuleRegisterBase> getModuleRegisterBase(const std::string& moduleName);

  /// Get all modules
  /// @return a vector with all the modules
  std::vector< Common::SafePtr<Common::ModuleRegisterBase> > getAllModules();

private: // data

  Common::GeneralStorage<Common::ModuleRegisterBase> m_store;

}; // ModuleRegistry

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ModuleRegistry_hpp
