// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CoreVars.hpp"
#include "Common/LibraryRegisterBase.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

CoreVars::CoreVars() :
  OnlyCPU0Writes       ( true  ),
  RegistSignalHandlers ( true  ),
  TraceActive          ( false ),
  TraceToStdOut        ( false ),
  VerboseEvents        ( false ),
  ErrorOnUnusedConfig  ( false ),
  MainLoggerFileName("output.log"),
  ExceptionLogLevel( (Uint) VERBOSE),
  InitArgs()
{
  InitArgs.first  = 0;
  InitArgs.second = CFNULL;
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF
