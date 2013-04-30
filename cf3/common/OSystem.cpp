// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cstdlib>  // provides system call
#include <stdlib.h>

#include "common/Assertions.hpp"
#include "common/OSystemLayer.hpp"
#include "common/StringConversion.hpp"

#ifdef CF3_HAVE_DLOPEN
  #include "common/PosixDlopenLibLoader.hpp"
#endif

#ifdef CF3_OS_LINUX
  #include "common/Linux/OSystemLayer.hpp"
#endif

#ifdef CF3_OS_MACOSX
  #include "common/MacOSX/OSystemLayer.hpp"
#endif

#ifdef CF3_OS_WINDOWS
  #include "common/Win32/OSystemLayer.hpp"
  #include "common/Win32/LibLoader.hpp"
#else
  
#endif

#include "common/OSystem.hpp"

#include "coolfluid-paths.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace cf3 {
  namespace common {

////////////////////////////////////////////////////////////////////////////////

OSystem::OSystem() :
  m_layer(),
  m_lib_loader()
{
  m_layer.reset();
  m_lib_loader.reset();

#ifdef CF3_HAVE_DLOPEN
    if ( is_null( m_lib_loader ) )   m_lib_loader.reset( new PosixDlopenLibLoader() );
#endif

#ifdef CF3_OS_LINUX
    if ( is_null( m_layer ) ) m_layer.reset( new Linux::OSystemLayer() );
#else
#ifdef CF3_OS_MACOSX
    if ( is_null( m_layer ) ) m_layer.reset( new MacOSX::OSystemLayer() );
#else
#ifdef CF3_OS_WINDOWS
    if ( is_null( m_layer ) ) m_layer.reset( new Win32::OSystemLayer() );
    if ( is_null( m_lib_loader ) )   m_lib_loader.reset( new Win32::LibLoader() );
#else
  #error "Unkown operating system: not Windows, MacOSX or Linux"
#endif
#endif
#endif

    cf3_assert ( is_not_null( m_layer ) );
    cf3_assert ( is_not_null( m_lib_loader   ) );
  
  std::vector< URI > default_paths(1, URI(CF3_BUILD_DIR,URI::Scheme::FILE) / URI("dso"));
  m_lib_loader->set_search_paths(default_paths);
}

OSystem::~OSystem()
{
}

OSystem& OSystem::instance()
{
  static OSystem osys;
  return osys;
}

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr<OSystemLayer> OSystem::layer()
{
  return m_layer;
}

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr<LibLoader> OSystem::lib_loader()
{
  return m_lib_loader;
}

////////////////////////////////////////////////////////////////////////////////

void OSystem::setenv(const string& name, const string& value)
{
#ifdef CF3_OS_WINDOWS
  ::putenv_s(name.c_str(), value.c_str());
#else
  ::setenv(name.c_str(), value.c_str(), true);
#endif
}

////////////////////////////////////////////////////////////////////////////////

  } // common
} // cf3
