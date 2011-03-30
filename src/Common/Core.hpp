// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Core_hpp
#define CF_Common_Core_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/shared_ptr.hpp>

#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  class EventHandler;
  class BuildInfo;
  class CodeProfiler;
  class CRoot;
  class CEnv;
  class CLibraries;
  class CFactories;
  class NetworkInfo;

////////////////////////////////////////////////////////////////////////////////

/// This class represents a singleton object where
/// which is used to initialize the CF runtime environment.
/// @author Tiago Quintino
class Common_API  Core : public boost::noncopyable {

public: // methods

  /// @return the instance of this singleton
  static Core& instance();

  /// Initializes the CF runtime enviroment.
  /// @pre Must be called prior to any CF runtime function,
  ///      only module registration procedures are allowed beforehand.
  /// @todo This is still broken for mpi as it doesn't allow modification of argc & argv
  void initiate(int argc, char** argv);

  /// Closes the CF runtime environment.
  /// @post Must not call any CF runtime functions after,
  ///       only destruction procedures ar allowed afterwards.
  void terminate();

  /// @brief Gives the root component.
  /// @return Returns the root component.
  boost::shared_ptr<CRoot> root() const;

  /// Gets the EventHandler of the CF runtime environment
  /// @pre Core does not need to be initialized before
  /// @post never nullptr
  boost::shared_ptr<Common::EventHandler> event_handler() const;

  /// Gets the BuildInfo
  /// @pre Core does not need to be initialized before
  /// @post never nullptr
  boost::shared_ptr<Common::BuildInfo> build_info() const;

  /// Gets the CEnv
  /// @pre Core does not need to be initialized before
  /// @post never nullptr
  boost::shared_ptr<Common::CEnv> environment() const;

  /// Gets the CLibraries
  /// @pre Core does not need to be initialized before
  /// @post never nullptr
  boost::shared_ptr<Common::CLibraries> libraries() const;

  /// Gets the CFactories
  /// @pre Core does not need to be initialized before
  /// @post never nullptr
  boost::shared_ptr<Common::CFactories> factories() const;

  /// @brief Sets the profiler.
  /// @param profiler_name Profiler name
  /// @throw ValueNotFound if no such profiler was found
  /// @todo Checks if another profiler has been set before
  void set_profiler(const std::string& profiler_name);

  /// @brief Gives the current profiler
  /// @return Returns the current profiler
  boost::shared_ptr<CodeProfiler> profiler() const;

  /// Gets the network information.
  /// @return Returns the network information.
  boost::shared_ptr<NetworkInfo> network_info() const;
  
  /// command-line arguments count
  /// @return count of arguments
  int argc() const { return m_argc; }
  
  /// command-line arguments values
  /// @return values of arguments
  char** argv() const { return m_argv; }

private: // methods

  /// Contructor
  Core();
  /// Destructor
  ~Core();

private: // data

  /// the EventHandler unique object
  boost::shared_ptr< Common::EventHandler > m_event_handler;
  /// the BuildInfo unique object
  boost::shared_ptr< Common::BuildInfo >    m_build_info;
  /// the CEnv unique object
  boost::shared_ptr< Common::CEnv >         m_environment;
  /// the CLibraries unique object
  boost::shared_ptr< Common::CLibraries >   m_libraries;
  /// the CFactories unique object
  boost::shared_ptr< Common::CFactories >   m_factories;
  /// @brief The component tree root
  boost::shared_ptr< Common::CRoot >        m_root;
  /// The network information
  boost::shared_ptr< Common::NetworkInfo >   m_network_info;

  /// command-line arguments count
  int m_argc;
  /// command-line arguments values
  char** m_argv;

}; // Core

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Core_hpp
