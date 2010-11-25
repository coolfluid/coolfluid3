// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cstdlib>  // provides system call

#include "Common/OSystemLayer.hpp"
#include "Common/String/Conversion.hpp"

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
  m_system_layer (nullptr),
  m_lib_loader(nullptr)
{

#ifdef CF_HAVE_DLOPEN
    if ( m_lib_loader == nullptr )   m_lib_loader = new PosixDlopenLibLoader();
#endif

#ifdef CF_OS_LINUX
    if ( m_system_layer == nullptr ) m_system_layer = new Linux::OSystemLayer();
#else
#ifdef CF_OS_MACOSX
    if ( m_system_layer == nullptr ) m_system_layer = new MacOSX::OSystemLayer();
#else
#ifdef CF_OS_WINDOWS
    if ( m_system_layer == nullptr ) m_system_layer = new Win32::OSystemLayer();
    if ( m_lib_loader == nullptr )   m_lib_loader   = new Win32::LibLoader();
#else
  #error "Unkown operating system: not Windows, MacOSX or Linux"
#endif
#endif
#endif

    cf_assert ( m_system_layer != nullptr);
    cf_assert ( m_lib_loader   != nullptr);

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

void OSystem::set_profiler(const std::string & profiler_name)
{
#error "Factory no longer exists - update this code"
  m_profiler = Factory<CodeProfiler>::instance().get_provider(profiler_name)->create();
}

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr<CodeProfiler> OSystem::profiler() const
{
  return m_profiler;
}


////////////////////////////////////////////////////////////////////////////////

  } // Common
} // CF
