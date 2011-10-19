// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include "Common/Log.hpp"

// dlopen header
#ifdef CF3_HAVE_DLOPEN
#  include <dlfcn.h>
#endif // CF3_HAVE_DLOPEN

#include "Common/BoostFilesystem.hpp"

#include "Common/PosixDlopenLibLoader.hpp"
#include "Common/CommonAPI.hpp"

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

void PosixDlopenLibLoader::set_search_paths(const std::vector< boost::filesystem::path >& paths)
{
  m_search_paths = paths;
}

////////////////////////////////////////////////////////////////////////////////

void* PosixDlopenLibLoader::call_dlopen(const boost::filesystem::path& fpath)
{
  using namespace boost::filesystem;

  void* hdl = nullptr;

  hdl = dlopen (fpath.string().c_str(), RTLD_LAZY|RTLD_GLOBAL);

  if( is_not_null(hdl) )
    CFinfo << "dlopen() loaded library \'" << fpath.string() << "\'" << CFendl;

  // library name
  if ( is_null(hdl) && !fpath.is_complete() )
  {

    // loop over the search paths and attempt to load the library

    std::vector< path >::const_iterator itr = m_search_paths.begin();
    for ( ; itr != m_search_paths.end(); ++itr)
    {
//          CFinfo << "searching in [" << *itr << "]\n" << CFflush;
      path fullqname = *itr / fpath;
//          CFinfo << "fullqname [" << fullqname.string() << "]\n" << CFflush;
      hdl = dlopen (fullqname.string().c_str(), RTLD_LAZY|RTLD_GLOBAL);
      if( hdl != nullptr )
      {
        CFinfo << "dlopen() loaded library \'" << fullqname.string() << "\'" << CFendl;
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

  boost::filesystem::path libpath( lib );

  // library handler
  void* hdl = nullptr;

  // try to load as passed ( still searches in paths )
  hdl = call_dlopen( libpath );
  if( is_not_null(hdl) ) return;

  // if failed, check if extension is correct then try again

  string filename = libpath.filename();
  path basepath = libpath.parent_path();

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

  hdl = call_dlopen( basepath / path(filewext) );

  // check for success
  if( is_not_null(hdl) ) return;

  // react on failure
  const char * msg = dlerror();
  throw LibLoadingError (FromHere(),"Library " + lib + " failed to load with dlopen error: " + std::string(msg));
}

////////////////////////////////////////////////////////////////////////////////

  } // common

} // cf3

////////////////////////////////////////////////////////////////////////////////
