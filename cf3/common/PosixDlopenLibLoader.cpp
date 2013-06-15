// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include "common/Log.hpp"

// dlopen header
#ifdef CF3_HAVE_DLOPEN
#  include <dlfcn.h>
#endif // cf3_HAVE_DLOPEN

//#include "common/BoostFilesystem.hpp"
#include "common/URI.hpp"
#include "common/PosixDlopenLibLoader.hpp"
#include "common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace cf3 {

  namespace common {

////////////////////////////////////////////////////////////////////////////////

PosixDlopenLibLoader::PosixDlopenLibLoader() : LibLoader()
{
}

////////////////////////////////////////////////////////////////////////////////

PosixDlopenLibLoader::~PosixDlopenLibLoader()
{
}

////////////////////////////////////////////////////////////////////////////////

void PosixDlopenLibLoader::set_search_paths(const std::vector< URI >& paths)
{
  m_search_paths = paths;
}

////////////////////////////////////////////////////////////////////////////////

void* PosixDlopenLibLoader::call_dlopen(const URI& fpath)
{
  void* hdl = nullptr;

  hdl = dlopen (fpath.path().c_str(), RTLD_LAZY|RTLD_GLOBAL);

  if( is_not_null(hdl) )
    CFdebug << "dlopen() loaded library \'" << fpath.path() << "\'" << CFendl;

  // library name
  if ( is_null(hdl) && !fpath.is_absolute() )
  {

    // loop over the search paths and attempt to load the library

    std::vector< URI >::const_iterator itr = m_search_paths.begin();
    for ( ; itr != m_search_paths.end(); ++itr)
    {
//          CFinfo << "searching in [" << *itr << "]\n" << CFflush;
      URI fullqname = *itr / fpath;
//          CFinfo << "fullqname [" << fullqname.string() << "]\n" << CFflush;
      hdl = dlopen (fullqname.path().c_str(), RTLD_LAZY|RTLD_GLOBAL);
      if( hdl != nullptr )
      {
        CFdebug << "dlopen() loaded library \'" << fullqname.path() << "\'" << CFendl;
        break;
      }
    }
  }

  return hdl; // will return nullptr if failed
}

////////////////////////////////////////////////////////////////////////////////

void PosixDlopenLibLoader::system_load_library(const std::string& lib)
{
  using namespace boost::filesystem;
  using namespace boost::algorithm;

  if (lib.empty()) return;

  URI libpath( lib );

  // library handler
  void* hdl = nullptr;

  // try to load as passed ( still searches in paths )
  hdl = call_dlopen( libpath );
  if( is_not_null(hdl) ) return;

  // if failed, check if extension is correct then try again

  std::string filename = libpath.name();
  URI basepath = libpath.base_path();

  std::string noext;
  std::string filewext;

  // get extension

  std::string::size_type n = filename.rfind('.');
  if (n != std::string::npos)
    noext = filename.substr(0,n);
  else
    noext = filename;

  // add library extention

#ifdef CF3_OS_LINUX
  filewext = noext + ".so";
#endif
#ifdef CF3_OS_MACOSX
  filewext += noext + ".dylib";
#endif
#ifdef CF3_OS_WINDOWS
  filewext += noext + ".dll";
#endif

  // also check if name starts with 'lib'
  if( !starts_with(filewext,"lib") )
    filewext = "lib" + filewext;

  hdl = call_dlopen( basepath / URI(filewext) );

  // check for success
  if( is_not_null(hdl) ) return;

  // react on failure
  const char * msg = dlerror();
  throw LibLoadingError ("Library " + lib + " failed to load with dlopen error: " + std::string(msg));
}

////////////////////////////////////////////////////////////////////////////////

  } // common

} // cf3

////////////////////////////////////////////////////////////////////////////////
