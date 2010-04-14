#ifndef CF_Common_ModuleRegisterBase_hh
#define CF_Common_ModuleRegisterBase_hh

////////////////////////////////////////////////////////////////////////////////

#include "Common/NonCopyable.hpp"

#include "Common/NamedObject.hpp"

//#include "Common/SelfRegistry.hpp"
//#include "Common/ConfigRegistry.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// This class represents a module register.
/// It holds all the registers for ConfigObject's and
/// inside a module self-registration objects.
/// The purpose is to announce to the framework the added functionality
/// and configuration options as a new module is loaded.
/// There should exist only one global instance of this object per
/// object.
/// @author Tiago Quintino
class Common_API ModuleRegisterBase :
  public Common::NonCopyable<ModuleRegisterBase>,
  public Common::NamedObject  {

public: // methods

  /// Gets the SelfRegistry of this Module Register
//  SelfRegistry& getSelfRegistry();

  /// Gets the ConfigRegistry of this Module Register
//  ConfigRegistry& getConfigRegistry();

  /// Returns the description of the module.
  /// Must be implemented by the ModuleRegister
  /// @return descripton of the module
  virtual std::string getDescription() const = 0;

  /// If needed initiates the module environment
  /// By default does nothing, it is meant to be overriden by the concrete classes
  /// @post m_init = true
  virtual void initiate();

  /// If needed initiates the module environment
  /// By default does nothing, it is meant to be overriden by the concrete classes
  /// @post m_init = false
  virtual void terminate();

  /// Check if this module has initialied its environment
  /// @return m_init
  bool isInitialized() const {  return m_init;  }

protected: // methods

    /// Constructor
  ModuleRegisterBase(const std::string& name);

    /// Destructor
  virtual ~ModuleRegisterBase();

protected: // data

  /// the SelfRegistry object is only held by the ModuleRegisterBase's
//  Common::SelfRegistry    m_selfRegistry;

  /// the ConfigRegistry object is only held by the ModuleRegisterBase's
//  Common::ConfigRegistry  m_configRegistry;

  /// is this module initialized
  bool m_init;

}; // end class ModuleRegisterBase

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ModuleRegisterBase_hh
