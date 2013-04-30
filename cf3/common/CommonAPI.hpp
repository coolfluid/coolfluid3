// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_CommonAPI_hpp
#define cf3_common_CommonAPI_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CF.hpp"

namespace cf3 {

/// @brief Most basic kernel library
///
/// Common holds classes that abstract the Operating System and the Parallel environment
/// and provide common facilities (like logging) to all other libraries.
/// Common is the most basic of the Kernel libraries.
namespace common {

/// Define the macro Common_API
/// @note build system defines COOLFLUID_COMMON_EXPORTS when compiling Common files
#ifdef COOLFLUID_COMMON_EXPORTS
#   define Common_API CF3_EXPORT_API
#   define Common_TEMPLATE
#else
#   define Common_API CF3_IMPORT_API
#   define Common_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#include "common/Tags.hpp"

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_CommonAPI_hpp
