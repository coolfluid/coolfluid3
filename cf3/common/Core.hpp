// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Core_hpp
#define cf3_common_Core_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

  class EventHandler;
  class BuildInfo;
  class CodeProfiler;
  class CRoot;
  class CEnv;
  class CGroup;
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
  common::CRoot& root() const;

  /// Gets the EventHandler of the CF runtime environment
  /// @pre Core does not need to be initialized before
  common::EventHandler& event_handler() const;

  /// Gets the BuildInfo
  /// @pre Core does not need to be initialized before
  common::BuildInfo& build_info() const;

  /// Gets the CEnv
  /// @pre Core does not need to be initialized before
  common::CEnv& environment() const;

  /// Gets the CLibraries
  /// @pre Core does not need to be initialized before
  common::CLibraries& libraries() const;

  /// Gets the CFactories
  /// @pre Core does not need to be initialized before
  common::CFactories& factories() const;

  /// Gets the tools
  /// @pre Core does not need to be initialized before
  common::CGroup& tools() const;

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
  NetworkInfo& network_info() const;
  
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

  /// the BuildInfo unique object
  boost::shared_ptr< common::BuildInfo >    m_build_info;
  /// the CEnv unique object
  boost::shared_ptr< common::CEnv >         m_environment;
  /// the CLibraries unique object
  boost::weak_ptr< common::CLibraries >   m_libraries;
  /// the CFactories unique object
  boost::weak_ptr< common::CFactories >   m_factories;
  /// @brief The component tree root
  boost::shared_ptr< common::CRoot >        m_root;
  /// The network information
  boost::shared_ptr< common::NetworkInfo >   m_network_info;

  /// command-line arguments count
  int m_argc;
  /// command-line arguments values
  char** m_argv;

}; // Core

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Core_hpp
