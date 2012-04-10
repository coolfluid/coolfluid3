// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_RegistLibrary_hpp
#define cf3_common_RegistLibrary_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Core.hpp"
#include "common/Libraries.hpp"
#include "common/Log.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

/// @brief Struct to force library registration
/// @author Quentin Gasper
template< typename LIB >
struct RegistLibrary
{
  /// @brief Registers the library LIB in the registry.
  RegistLibrary()
  {
    CFdebug << "Library [" << Core::instance().libraries().library<LIB>()->type_name() << "] loaded." << CFendl;

//    Core::instance().libraries().library<LIB>()->initiate();

    Core::instance().libraries().library<LIB>();

  }

//  ~RegistLibrary()
//  {
////    CFinfo << "Library [" << Core::instance().libraries().library<LIB>()->type_name() << "] unloaded." << CFendl;

//    // should not do anything
//    Core::instance().libraries().library<LIB>()->terminate();
//  }

};

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_RegistLibrary_hpp
