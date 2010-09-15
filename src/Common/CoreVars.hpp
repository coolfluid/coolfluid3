// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CoreVars_hpp
#define CF_Common_CoreVars_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "Common/LogLevel.hpp"
#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Stores CF Runtime environment variables
/// @author Tiago Quintino
class Common_API CoreVars : public boost::noncopyable
{
  public: // functions

    /// Constructor
    CoreVars();

  public: // data

    /// only processor P0 outputs the log info to files
    bool OnlyCPU0Writes;
    /// assertions throw exceptions instead of aborting
    bool AssertionThrows;
    /// regist signal handlers
    bool RegistSignalHandlers;
    /// activate trace
    bool TraceActive;
    /// tracing also sento to StdOut to be put into CoreEnv
    bool TraceToStdOut;
    /// If Events have verbose output
    bool VerboseEvents;
    /// Signal error when some user provided config parameters are not used
    bool ErrorOnUnusedConfig;
    /// the name of the file in which to put the logging messages
    std::string MainLoggerFileName;
    /// the loglevel for exceptions
    Uint ExceptionLogLevel;
    /// the initial arguments with which the environment was started
    std::pair<int,char**> InitArgs;

}; // end class CoreVars

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CoreVars_hpp
