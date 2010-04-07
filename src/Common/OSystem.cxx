#include <cstdlib>  // provides system call

#include "Common/ProcessInfo.hh" 
#include "Common/SignalHandler.hh"
#include "Common/StringOps.hh"

#ifdef CF_HAVE_DLOPEN
  #include "Common/PosixDlopenLibLoader.hh"
#endif

#ifdef CF_OS_LINUX
  #include "Common/Linux/ProcessInfo.hh"
  #include "Common/Linux/SignalHandler.hh"
#endif

#ifdef CF_OS_MACOSX
  #include "Common/MacOSX/ProcessInfo.hh"
  #include "Common/MacOSX/SignalHandler.hh"
#endif

#ifdef CF_OS_WINDOWS
  #include "Common/Win32/ProcessInfo.hh"
  #include "Common/Win32/SignalHandler.hh"
  #include "Common/Win32/LibLoader.hh"
#endif

#include "Common/OSystem.hh"

//////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace CF {
  namespace Common {

//////////////////////////////////////////////////////////////////////////////

OSystemError::OSystemError ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "OSystemError")
{}

//////////////////////////////////////////////////////////////////////////////

OSystem::OSystem() :
  m_process_info (CFNULL),
  m_sig_handler(CFNULL),
  m_lib_loader(CFNULL)
{

#ifdef CF_HAVE_DLOPEN
    if ( m_lib_loader == CFNULL )   m_lib_loader = new PosixDlopenLibLoader();
#endif

#ifdef CF_OS_LINUX
    if ( m_process_info == CFNULL ) m_process_info = new Linux::ProcessInfo();
    if ( m_sig_handler == CFNULL )  m_sig_handler  = new Linux::SignalHandler();
#else
#ifdef CF_OS_MACOSX
    if ( m_process_info == CFNULL ) m_process_info = new MacOSX::ProcessInfo();
    if ( m_sig_handler == CFNULL )  m_sig_handler  = new MacOSX::SignalHandler();
#else
#ifdef CF_OS_WINDOWS
    if ( m_process_info == CFNULL ) m_process_info = new Win32::ProcessInfo();
    if ( m_sig_handler == CFNULL )  m_sig_handler  = new Win32::SignalHandler();
    if ( m_lib_loader == CFNULL )   m_lib_loader   = new Win32::LibLoader();
#else
  #error "Unkown operating system: not Windows, MacOSX or Linux"
#endif
#endif
#endif

    cf_assert ( m_process_info != CFNULL);
    cf_assert ( m_sig_handler  != CFNULL);
    cf_assert ( m_lib_loader   != CFNULL);

}

//////////////////////////////////////////////////////////////////////////////

OSystem::~OSystem()
{
  delete_ptr (m_process_info);
  delete_ptr (m_sig_handler);
  delete_ptr (m_lib_loader);
}

//////////////////////////////////////////////////////////////////////////////

OSystem& OSystem::getInstance()
{
  static OSystem osystem;
  return osystem;
}

//////////////////////////////////////////////////////////////////////////////

SafePtr<ProcessInfo> OSystem::getProcessInfo()
{
  return m_process_info;
}

//////////////////////////////////////////////////////////////////////////////

SafePtr<SignalHandler> OSystem::getSignalHandler()
{
  return m_sig_handler;
}

//////////////////////////////////////////////////////////////////////////////

SafePtr<LibLoader> OSystem::getLibLoader()
{
  return m_lib_loader;
}

//////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////

void OSystem::sleep (const CF::Uint& seconds)
{
  std::string callSleep = "sleep " + Common::StringOps::to_str(seconds);
  executeCommand (callSleep);
}

//////////////////////////////////////////////////////////////////////////////

  } // namespace Common
} // namespace CF
