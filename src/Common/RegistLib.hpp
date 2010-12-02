// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_RegistLib_hpp
#define CF_Common_RegistLib_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Core.hpp"
#include "Common/CLibraries.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// @brief Struct to force library registration
/// @author Quentin Gasper
template< typename LIB >
struct ForceLibRegist
{
  /// @brief Registers the library LIB in the registry.
  ForceLibRegist()
  {
    // CFinfo << "Library [" << Core::instance().libraries()->get_library<LIB>()->type_name() << "] loaded." << CFendl;

    Core::instance().libraries()->get_library<LIB>();
  }
};

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_RegistLib_hpp
