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
#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  class EventHandler;
  class BuildInfo;
  class CodeProfiler;
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

  /// Gets the EventHandler of the CF runtime environment
  /// @note Does not need to be initialized before
  boost::weak_ptr<Common::EventHandler> event_handler();

  /// Gets the BuildInfo
  /// @note Does not need to be initialized before
  boost::weak_ptr<Common::BuildInfo> build_info();

  /// @brief Sets the profiler.
  /// @param profiler_name Profiler name
  /// @throw ValueNotFound if no such profiler was found
  /// @todo Checks if another profiler has been set before
  void set_profiler(const std::string& profiler_name);

  /// @brief Gives the current profiler
  /// @return Returns the current profiler
  boost::shared_ptr<CodeProfiler> profiler() const;

private: // methods

  /// Default contructor
  Core();
  /// Default destructor
  ~Core();

private: // data

  /// the EventHandler object is only held by the CoreEnv singleton object
  boost::shared_ptr< Common::EventHandler > m_event_handler;

  /// the BuildInfo object
  boost::shared_ptr< Common::BuildInfo > m_build_info;

  /// @brief The component tree root
  CRoot::Ptr m_root;

}; // Core

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
  // CFinfo << VERBOSE << "Library [" << LIB::instance().name() << "] loaded." << CFendl;
  LIB::instance();
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Core_hpp
