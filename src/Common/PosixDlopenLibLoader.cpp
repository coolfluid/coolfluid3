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
  CFLog( VERBOSE, "LinuxLibtoolLibLoader: Attempting to load " << lib << "\n" );

  using boost::filesystem::path;

// library name
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

  // library handler
  void* hdl = NULL;


  // loop over the searhc paths ans
  // attempt to load the library
  std::vector< path >::const_iterator itr = m_search_paths.begin();

  for (; itr != m_search_paths.end() ; ++itr)
  {
//    CFout << "searching in [" << *itr << "]\n" << CFflush;
    path fullqname = *itr / path(libname);
//    CFout << "fullqname [" << fullqname.string() << "]\n" << CFflush;
    hdl = dlopen (fullqname.string().c_str(), RTLD_LAZY|RTLD_GLOBAL);
    if( hdl != NULL ) break;
  }

  // check for success
  if(hdl != NULL)
  {
    CFLog( NORMAL, "PosixDlopenLibLoader: Loaded " << libname  << "\n" );
  }
  else
  {
    CFLog( NORMAL, "dlopen() failed to load module : " << libname  << "\n" );
    const char * msg = dlerror();
    if (msg != NULL)
    {
      CFLog( NORMAL, "dlerror() says : " << msg  << "\n" );
    }
    else
    {
      CFLog( NORMAL, "dlerror() said nothing." << "\n" );
    }
    throw LibLoadingError (FromHere(),"Module failed to load");
  }
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////
