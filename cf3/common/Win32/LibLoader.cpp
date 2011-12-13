// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cstdlib>

#include "common/Log.hpp" // CF_HAVE_WINDOWSH is defined via this header

// windows header ( maybe this define should be in the coolfluid_config.h )
#define _WIN32_WINNT 0x0502 // minimum requirement is ( "Windows Server 2003 with SP1" ) or ( "Windows XP with SP2" )
#include <windows.h>

#include "common/Win32/LibLoader.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
  namespace common {
  namespace Win32 {

////////////////////////////////////////////////////////////////////////////////

LibLoader::LibLoader()
{
}

////////////////////////////////////////////////////////////////////////////////

LibLoader::~LibLoader()
{
}

////////////////////////////////////////////////////////////////////////////////

void LibLoader::set_search_paths(const std::vector< boost::filesystem::path >& paths)
{
  m_search_paths = paths;
}

////////////////////////////////////////////////////////////////////////////////

void LibLoader::load_library(const std::string& lib)
{
  CFLog( VERBOSE , "LibLoader: Attempting to load '" << lib << "'\n" );
  using boost::filesystem::path;
  std::string libname = lib + ".dll";

  // attempt to load a module
  HINSTANCE hdl = nullptr;
  std::vector<path>::const_iterator itr = m_search_paths.begin();
  for (; itr != m_search_paths.end() ; ++itr)
  {
    // set the current search directory
    CFLog ( VERBOSE, "searching in dir: '" <<  (*itr).string() << "'\n" );
  SetDllDirectory((*itr).string().c_str());

  // try to load the library
    hdl = LoadLibrary(TEXT(libname.c_str()));
    if( hdl != nullptr ) break;
    CFLog ( VERBOSE, "didnt find '" <<  lib << "' in dir '" <<  (*itr).string() << "'\n" );
  }

  // searhc in the global path
  hdl = LoadLibrary(TEXT(libname.c_str()));
  if( hdl == nullptr )
  {
    char * pPath;
    pPath = getenv ("PATH");
    if (pPath != nullptr)
      CFLog ( VERBOSE, "didnt find '" <<  lib << "' in global path variable PATH='" << pPath << "'\n" );
  }

  // check for success
  if(hdl != NULL)
  {
    CFLog( VERBOSE, "LibLoader: Loaded '" << libname << "'\n" );
  }
  else
  {
    CFLog( VERBOSE, "LoadLibrary() failed to load module : '" << libname << "'\n" );
    throw LibLoadingError ("Module failed to load '" + libname + "'"  );
  }
}

////////////////////////////////////////////////////////////////////////////////

} // Win32
  } // namespace common
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////

