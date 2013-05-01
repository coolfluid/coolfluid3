// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_LogLevel_hpp
#define cf3_common_LogLevel_hpp

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

////////////////////////////////////////////////////////////////////////////////

/// Output levels for the Log facility
/// @author Quentin Gasper
/// @author Willem Deconinck

enum LogLevel
{
  SILENT   = 0, // output empty
  ERROR    = 1, // output errors
  WARNING  = 2, // output errors and warnings
  INFO     = 3, // output errors and warnings and info
  DEBUG    = 4, // output errors and warnings and info and debug
  VERBOSE  = 10 // output everything
}; // LogLevel



////////////////////////////////////////////////////////////////////////////////

} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_LogLevel_hpp
