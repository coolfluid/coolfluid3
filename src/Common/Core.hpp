// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Core_hpp
#define CF_Common_Core_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/shared_ptr.hpp>

#include "Common/CF.hpp"
#include "Common/Log.hpp"

#include "Common/LibraryRegistry.hpp"
#include "Common/SafePtr.hpp"

#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  class EventHandler;
  class FactoryRegistry;
  class BuildInfo;
  class CRoot;

////////////////////////////////////////////////////////////////////////////////

/// This class represents a singleton object where
/// which is used to initialize the CF runtime environment.
/// @author Tiago Quintino
class Common_API  Core : public boost::noncopyable {

public: // methods

  /// @return the instance of this singleton
  static Core& instance();

  /// @brief Gives the root component.
  /// @return Returns the root component.
  boost::shared_ptr<CRoot> root();

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

  /// Gets the LibraryRegistry
  /// @note Does not need to be initialized before
  boost::weak_ptr<Common::LibraryRegistry> library_registry();

  /// Gets the FactoryRegistry
  /// @note Does not need to be initialized before
  boost::weak_ptr<Common::FactoryRegistry> factory_registry();

  /// Gets the EventHandler of the CF runtime environment
  /// @note Does not need to be initialized before
  boost::weak_ptr<Common::EventHandler> event_handler();

  /// Gets the BuildInfo
  /// @note Does not need to be initialized before
  boost::weak_ptr<Common::BuildInfo> build_info();

private: // methods

  /// Default contructor
  Core();
  /// Default destructor
  ~Core();

private: // data

  /// the EventHandler object is only held by the CoreEnv singleton object
  boost::shared_ptr< Common::EventHandler > m_event_handler;

  /// the LibraryRegistry singleton object is only held by the CoreEnv singleton object
  boost::shared_ptr< Common::LibraryRegistry > m_module_registry;

  /// the FactoryRegistry singleton object is only held by the CoreEnv singleton object
  boost::shared_ptr< Common::FactoryRegistry > m_factory_registry;

  /// the BuildInfo object
  boost::shared_ptr< Common::BuildInfo > m_build_info;

  /// @brief The component tree root
  boost::shared_ptr<CRoot> m_root;

}; // CoreEnv

////////////////////////////////////////////////////////////////////////////////

/// @brief Struct to force library registration
/// @author Quentin Gasper
template< typename LIB >
struct ForceLibRegist
{
  /// @brief Registers the library LIB in the registry.
  ForceLibRegist();
};

////////////////////////////////////////////////////////////////////////////////

template < typename LIB >
ForceLibRegist<LIB>::ForceLibRegist()
{
  CFinfo << VERBOSE << "Library [" << LIB::instance().getName() << "] loaded." << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Core_hpp
