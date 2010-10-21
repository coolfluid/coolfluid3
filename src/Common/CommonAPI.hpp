// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CommonAPI_hpp
#define CF_Common_CommonAPI_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"

namespace CF {

  /// Common holds classes that abstract the Operating System and the Parallel environment
  /// and provide common facilities (like logging) to all other libraries.
  /// Common is the most basic of the Kernel libraries.
  namespace Common {

  /// Define the macro Common_API
  /// @note build system defines COOLFLUID_COMMON_EXPORTS when compiling Common files
  #ifdef COOLFLUID_COMMON_EXPORTS
  #   define Common_API CF_EXPORT_API
  #else
  #   define Common_API CF_IMPORT_API
  #endif

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CommonAPI_hpp
