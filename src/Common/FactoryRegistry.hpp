#ifndef CF_Common_FactoryRegistry_hpp
#define CF_Common_FactoryRegistry_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/SafePtr.hpp"
#include "Common/GeneralStorage.hpp"
#include "Common/NonCopyable.hpp"

#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

    class FactoryBase;
    class CoreEnv;

////////////////////////////////////////////////////////////////////////////////

/// This class is a singleton object which serves as registry for all the
/// Factory objects that are created
/// The only instance of this object is held by the CoreEnv.
/// @author Tiago Quintino
class Common_API FactoryRegistry :
  public Common::NonCopyable<FactoryRegistry> {

  friend class Common::CoreEnv;

public:

  /// Register a factory
  /// @param module pointer to a FactoryBase to be added
  void regist(Common::FactoryBase* factory);

  /// Remove a registered factory
  /// @param name name of a FactoryBase to be removed
  void unregist(const std::string& name);

  /// Get a given factory
  /// @param name name of a FactoryBase to be accessed
  /// @return a pointer to a FactoryBase if found or a null pointer if not found
  Common::SafePtr<Common::FactoryBase> getFactory(const std::string& name);

private: // methods

  /// Constructor is private to allow only the friend classes to build it
  FactoryRegistry();

  /// Default destructor is private to allow only the friend classes to destroy it
  ~FactoryRegistry();

private: // data

  Common::GeneralStorage<Common::FactoryBase> m_store;

}; // end of class FactoryRegistry

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_FactoryRegistry_hpp
