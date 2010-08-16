#ifndef CF_Common_CoreEnv_hpp
#define CF_Common_CoreEnv_hpp

////////////////////////////////////////////////////////////////////////////////

#include "boost/checked_delete.hpp"

#include "Common/CF.hpp"

#include "Common/SafePtr.hpp"
#include "Common/SetupObject.hpp"
#include "Common/SharedPtr.hpp"

#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  class EventHandler;
  class ModuleRegistry;
  class FactoryRegistry;
  class CoreVars;

////////////////////////////////////////////////////////////////////////////////

/// This class represents a singleton object where
/// which is used to initialize the CF runtime environment.
/// @author Tiago Quintino
class Common_API  CoreEnv :
    public boost::noncopyable,
//  public Common::ConfigObject,
  public Common::SetupObject {

public: // methods

  /// @return the instance of this singleton
  static CoreEnv& instance();

  /// Defines the Config Option's of this class
  /// @param options a OptionList where to add the Option's
//  static void defineConfigOptions(Common::OptionList& options);

  /// Configures this Simulator.
  /// Sets up the data for this Object.
//  virtual void configure ( Common::ConfigArgs& args );

  /// Setup the environment
  /// @pre called after configure
  virtual void setup();
  /// Unsetup the object
  /// @pre called after setup
  virtual void unsetup();

  /// Initializes the CF runtime enviroment.
  /// @pre Must be called prior to any CF runtime function,
  ///      only module registration procedures are allowed beforehand.
  /// @TODO This is still broken for mpi as it doesn't allow modification
  ///  of argc & argv
  void initiate(int argc, char** argv);

  /// Closes the CF runtime environment.
  /// @post Must not call any CF runtime functions after,
  ///       only destruction procedures ar allowed afterwards.
  void terminate();

  /// Gets the ModuleRegistry
  /// @note Does not need to be initialized before
  Common::SafePtr<Common::ModuleRegistry> getModuleRegistry();

  /// Gets the FactoryRegistry
  /// @note Does not need to be initialized before
  Common::SafePtr<Common::FactoryRegistry> getFactoryRegistry();

  /// Gets the EventHandler of the CF runtime environment
  /// @note Does not need to be initialized before
  Common::SafePtr<Common::EventHandler> getEventHandler();

  /// Gets the CoreVars
  Common::SafePtr<Common::CoreVars> getVars();

  /// Calls initiate() on all registered modules.
  /// Mind that some modules might already have been initiated.
  /// It is up to the modules to track if they have or not been initiated.
  /// @see ModuleRegisterBase
  void initiate_modules();

  /// Calls terminate() on all registered modules
  /// Mind that some modules might already have been terminated.
  /// It is up to the modules to track if they have or not been terminated.
  /// @see ModuleRegisterBase
  void terminate_modules();

  /// Return the subversion version string of this build
  std::string getVersionHeader() const;
  /// Return the subversion version string of this build
  std::string getSvnVersion() const;
  /// Return the CF version string
  std::string getReleaseVersion() const;
  /// Return the CF Kernel version string
  std::string getKernelVersion() const;
  /// Return the CF build type
  std::string getBuildType() const;
  /// Return the CMake version
  std::string getBuildSystem() const;
  /// Return the build processor
  std::string getBuildProcessor() const;
  /// OS short name. Examples: "Linux" or "Windows"
  /// @return string with short OS name
  std::string getSystemName() const;
  /// OS short name. Examples: "Linux-2.6.23" or "Windows 5.1"
  /// @return string with long OS name
  std::string getLongSystemName() const;
  /// OS version. Examples: "2.6.23" or "5.1"
  /// @return string with OS version
  std::string getSystemVersion() const;
  /// OS bits. Examples: "32" or "64"
  /// @post should be equal to 8 * size_of(void*) but it given by the build system
  /// @return string with OS addressing size
  std::string getSystemBits() const;

private: // methods

  /// Default contructor
  CoreEnv();
  /// Default destructor
  ~CoreEnv();

private: // data

  /// the EventHandler object is only held by the CoreEnv singleton object
  Common::SharedPtr<Common::EventHandler> m_event_handler;

  /// the ModuleRegistry singleton object is only held by the CoreEnv singleton object
  Common::ModuleRegistry* m_module_registry;

  /// the FactoryRegistry singleton object is only held by the CoreEnv singleton object
  Common::FactoryRegistry* m_factory_registry;

  /// @brief Static environment variables
  /// pointer to a struct of variables that always exist
  CoreVars * m_env_vars;

}; // end of class CoreEnv

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CoreEnv_hpp
