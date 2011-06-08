// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_RegistLibrary_hpp
#define CF_Common_RegistLibrary_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Core.hpp"
#include "Common/CLibraries.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// @brief Struct to force library registration
/// @author Quentin Gasper
template< typename LIB >
struct RegistLibrary
{
  /// @brief Registers the library LIB in the registry.
  RegistLibrary()
  {
    //CFinfo << "Library [" << Core::instance().libraries().library<LIB>()->type_name() << "] loaded." << CFendl;

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

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_RegistLibrary_hpp
