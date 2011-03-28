// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/LibLoader.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

LibLoader::LibLoader()
{
}

LibLoader::~LibLoader()
{
}

////////////////////////////////////////////////////////////////////////////////

void LibLoader::load_library(const std::string& lib)
{
//  void* handle =
      system_load_library(lib);

  // place the pointer in the registered library
  // Core::instance().libraries()->get_library<LIB>()->initiate();

  // initiate all libraries not yet initiated
  // because loading one library might implicitly load many others

  // Core::instance().libraries()->get_library<LIB>()->initiate();

}

////////////////////////////////////////////////////////////////////////////////

void LibLoader::unload_library( CLibrary::Ptr lib )
{
  /// @todo terminate library to be unloaded

  // Core::instance().libraries()->get_library<LIB>()->initiate();

  //  system_unload_library(lib);
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////
