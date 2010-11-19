// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_LibraryRegistry_hpp
#define CF_Common_LibraryRegistry_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/SafePtr.hpp"
#include "Common/GeneralStorage.hpp"

#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

    class LibraryRegisterBase;

////////////////////////////////////////////////////////////////////////////////

/// This class is a singleton object which serves as registry for all the
/// modules that are loaded
/// The only instance of this object is held by the CFEnv.
/// @author Tiago Quintino
class Common_API LibraryRegistry : public boost::noncopyable {

public:

  /// Register a module
  /// @param module pointer to a LibraryRegisterBase to be added
  void regist(Common::LibraryRegisterBase* module);

  /// Remove a registered module
  /// @param moduleName name of a LibraryRegisterBase to be removed
  void unregist(const std::string& moduleName);

  /// Checks that a module is registered
  /// @param moduleName name of a LibraryRegisterBase to be checked
  bool isRegistered(const std::string& moduleName);

  /// Get a given module
  /// @param moduleName name of a LibraryRegisterBase to be accessed
  /// @return a pointer to a LibraryRegisterBase if found or a null pointer if not found
  Common::SafePtr<Common::LibraryRegisterBase> getLibraryRegisterBase(const std::string& moduleName);

  /// Get all modules
  /// @return a vector with all the modules
  std::vector< Common::SafePtr<Common::LibraryRegisterBase> > getAllModules();

private: // data

  Common::GeneralStorage<Common::LibraryRegisterBase> m_store;

}; // LibraryRegistry

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_LibraryRegistry_hpp
