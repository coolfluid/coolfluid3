// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"

// dlopen header
#ifdef CF_HAVE_DLOPEN
#  include <dlfcn.h>
#endif // CF_HAVE_DLOPEN

#include <boost/filesystem/path.hpp>

#include "Common/PosixDlopenLibLoader.hpp"
#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

PosixDlopenLibLoader::PosixDlopenLibLoader() : LibLoader()
{
}

////////////////////////////////////////////////////////////////////////////////

PosixDlopenLibLoader::~PosixDlopenLibLoader()
{
}

////////////////////////////////////////////////////////////////////////////////

void PosixDlopenLibLoader::set_search_paths(std::vector< boost::filesystem::path >& paths)
{
  m_search_paths = paths;
}

////////////////////////////////////////////////////////////////////////////////

void PosixDlopenLibLoader::load_library(const std::string& lib)
{
  if (lib.empty()) return;

  CFinfo << "dlopen() loading library \'" << lib << "\'" << CFendl;

  using namespace boost::filesystem;

  // library path
  boost::filesystem::path fpath ( lib );
  // library handler
  void* hdl = NULL;


  // library name
  if ( fpath.is_complete() )
  {
    hdl = dlopen (fpath.string().c_str(), RTLD_LAZY|RTLD_GLOBAL);
  }
  else
  {
    std::string libname = "lib" + lib;
    // add library extention
  #ifdef CF_OS_LINUX
    libname += ".so";
  #endif
  #ifdef CF_OS_MACOSX
    libname += ".dylib";
  #endif
  #ifdef CF_OS_WINDOWS
    libname += ".dll";
  #endif

    // loop over the searhc paths and attempt to load the library
    std::vector< path >::const_iterator itr = m_search_paths.begin();
    for (; itr != m_search_paths.end() ; ++itr)
    {
      //    CFout << "searching in [" << *itr << "]\n" << CFflush;
      path fullqname = *itr / path(libname);
      //    CFout << "fullqname [" << fullqname.string() << "]\n" << CFflush;
      hdl = dlopen (fullqname.string().c_str(), RTLD_LAZY|RTLD_GLOBAL);
      if( hdl != NULL ) break;
    }
  }

  // check for success
  if(hdl != NULL)
  {
    CFinfo << "dlopen(): loaded library \'" << lib  << "\'" << CFendl;
  }
  else
  {
    CFinfo << "dlopen() failed to load library : \'" << lib << "\'" << CFendl;
    const char * msg = dlerror();
    if (msg != NULL)
    {
      CFinfo << "dlerror() says : " << msg  <<  CFendl;
    }
    else
    {
      CFinfo << "dlerror() said nothing." << CFendl;
    }
    throw LibLoadingError (FromHere(),"Library failed to load");
  }
}

////////////////////////////////////////////////////////////////////////////////

  } // Common

} // CF

////////////////////////////////////////////////////////////////////////////////
