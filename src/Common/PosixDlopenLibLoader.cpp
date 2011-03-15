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

#include "Common/BoostFilesystem.hpp"

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

void PosixDlopenLibLoader::set_search_paths(const std::vector< boost::filesystem::path >& paths)
{
  m_search_paths = paths;
}

////////////////////////////////////////////////////////////////////////////////

void* PosixDlopenLibLoader::call_dlopen(const boost::filesystem::path& fpath)
{
  using namespace boost::filesystem;

  void* hdl = nullptr;

  CFinfo << "dlopen() loading library \'" << fpath.string() << "\'" << CFendl;

  // library name
  if ( fpath.is_complete() )
  {
    hdl = dlopen (fpath.string().c_str(), RTLD_LAZY|RTLD_GLOBAL);
  }
  else
  {
    // loop over the search paths and attempt to load the library
    std::vector< path >::const_iterator itr = m_search_paths.begin();
    for (; itr != m_search_paths.end() ; ++itr)
    {
      //    CFout << "searching in [" << *itr << "]\n" << CFflush;
      path fullqname = *itr / fpath;
      //    CFout << "fullqname [" << fullqname.string() << "]\n" << CFflush;
      hdl = dlopen (fullqname.string().c_str(), RTLD_LAZY|RTLD_GLOBAL);
      if( hdl != nullptr ) break;
    }
  }
  return hdl; // will return nullptr if failed
}

////////////////////////////////////////////////////////////////////////////////

void PosixDlopenLibLoader::load_library(const std::string& lib)
{
  using namespace boost::filesystem;

  if (lib.empty()) return;

  boost::filesystem::path libpath( lib );

  // library handler
  void* hdl = nullptr;

  // try to load as passed ( still searches in paths )
  hdl = call_dlopen( libpath );
  if( is_not_null(hdl) ) return;

  // if failed, check if extension is correct then try again

  std::string filename = libpath.filename();
  std::string noext;
  std::string filewext;

  // get extension

  std::string::size_type n = filename.rfind('.');
  if (n != std::string::npos)
    noext = filename.substr(0,n);
  else
    noext = filename;

  // add library extention

#ifdef CF_OS_LINUX
  filewext = noext + ".so";
#endif
#ifdef CF_OS_MACOSX
  filewext += noext + ".dylib";
#endif
#ifdef CF_OS_WINDOWS
  filewext += noext + ".dll";
#endif

  CFinfo << "dlopen() loading library \'" << filewext << "\'" << CFendl;

  hdl = call_dlopen( libpath );

  // check for success
  if( is_not_null(hdl) ) return;

  // react on failure

  CFerror << "dlopen() failed to load library : \'" << lib << "\'" << CFendl;
  const char * msg = dlerror();
  if ( is_null(msg) )
    CFerror << "dlerror() said nothing." << CFendl;
  else
    CFerror << "dlerror() says : " << msg  <<  CFendl;

  throw LibLoadingError (FromHere(),"Library failed to load");

}

////////////////////////////////////////////////////////////////////////////////

  } // Common

} // CF

////////////////////////////////////////////////////////////////////////////////
