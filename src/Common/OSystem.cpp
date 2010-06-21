#include <cstdlib>  // provides system call

#include "Common/OSystemLayer.hpp"
#include "Common/StringOps.hpp"

#ifdef CF_HAVE_DLOPEN
  #include "Common/PosixDlopenLibLoader.hpp"
#endif

#ifdef CF_OS_LINUX
  #include "Common/Linux/OSystemLayer.hpp"
#endif

#ifdef CF_OS_MACOSX
  #include "Common/MacOSX/OSystemLayer.hpp"
#endif

#ifdef CF_OS_WINDOWS
  #include "Common/Win32/OSystemLayer.hpp"
  #include "Common/Win32/LibLoader.hpp"
#endif

#include "Common/OSystem.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace CF {
  namespace Common {

////////////////////////////////////////////////////////////////////////////////

OSystemError::OSystemError ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "OSystemError")
{}

////////////////////////////////////////////////////////////////////////////////

OSystem::OSystem() :
  m_system_layer (CFNULL),
  m_lib_loader(CFNULL)
{

#ifdef CF_HAVE_DLOPEN
    if ( m_lib_loader == CFNULL )   m_lib_loader = new PosixDlopenLibLoader();
#endif

#ifdef CF_OS_LINUX
    if ( m_system_layer == CFNULL ) m_system_layer = new Linux::OSystemLayer();
#else
#ifdef CF_OS_MACOSX
    if ( m_system_layer == CFNULL ) m_system_layer = new MacOSX::OSystemLayer();
#else
#ifdef CF_OS_WINDOWS
    if ( m_system_layer == CFNULL ) m_system_layer = new Win32::OSystemLayer();
    if ( m_lib_loader == CFNULL )   m_lib_loader   = new Win32::LibLoader();
#else
  #error "Unkown operating system: not Windows, MacOSX or Linux"
#endif
#endif
#endif

    cf_assert ( m_system_layer != CFNULL);
    cf_assert ( m_lib_loader   != CFNULL);

}

////////////////////////////////////////////////////////////////////////////////

OSystem::~OSystem()
{
  delete_ptr (m_system_layer);
  delete_ptr (m_lib_loader);
}

////////////////////////////////////////////////////////////////////////////////

OSystem& OSystem::instance()
{
  static OSystem osys;
  return osys;
}

////////////////////////////////////////////////////////////////////////////////

SafePtr<OSystemLayer> OSystem::OSystemLayer()
{
  return m_system_layer;
}

////////////////////////////////////////////////////////////////////////////////

SafePtr<LibLoader> OSystem::LibLoader()
{
  return m_lib_loader;
}

////////////////////////////////////////////////////////////////////////////////

void OSystem::executeCommand(const std::string& call)
{
  int return_value = system ( call.c_str() );

  if ( return_value == -1)
  {
    std::string msg;
    msg += "Command \'";
    msg += call;
    msg += "\' return error code";
    throw OSystemError ( FromHere(), msg );
  }
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common
} // namespace CF
