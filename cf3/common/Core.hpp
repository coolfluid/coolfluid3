// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Core_hpp
#define cf3_common_Core_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/shared_ptr.hpp>

#include "common/CommonAPI.hpp"
#include "common/Handle.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

  class Component;
  class EventHandler;
  class BuildInfo;
  class CodeProfiler;
  class Environment;
  class Group;
  class Libraries;
  class Factories;
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

  /// @brief Gives the default root component.
  /// @return Returns the root component.
  common::Component& root() const;

  /// Gets the EventHandler of the CF runtime environment
  /// @pre Core does not need to be initialized before
  common::EventHandler& event_handler() const;

  /// Gets the BuildInfo
  /// @pre Core does not need to be initialized before
  common::BuildInfo& build_info() const;

  /// Gets the Environment
  /// @pre Core does not need to be initialized before
  common::Environment& environment() const;

  /// Gets the Libraries
  /// @pre Core does not need to be initialized before
  common::Libraries& libraries() const;

  /// Gets the Factories
  /// @pre Core does not need to be initialized before
  common::Factories& factories() const;

  /// Gets the tools
  /// @pre Core does not need to be initialized before
  common::Group& tools() const;

  /// @brief Sets the profiler.
  /// @param profiler_name Profiler name
  /// @throw ValueNotFound if no such profiler was found
  /// @todo Checks if another profiler has been set before
  void set_profiler(const std::string& profiler_name);

  /// @brief Gives the current profiler
  /// @return Returns the current profiler
  Handle<CodeProfiler> profiler() const;

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
  /// the Environment unique object
  boost::shared_ptr< common::Environment >  m_environment;
  /// the Libraries unique object
  boost::shared_ptr< common::Libraries >    m_libraries;
  /// the Factories unique object
  boost::shared_ptr< common::Factories >    m_factories;
  /// @brief The component tree root
  boost::shared_ptr< common::Group >        m_root;
  /// The network information
  boost::shared_ptr< common::NetworkInfo >  m_network_info;

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
