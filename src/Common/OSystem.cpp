// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cstdlib>  // provides system call

#include "Common/Assertions.hpp"
#include "Common/OSystemLayer.hpp"
#include "Common/StringConversion.hpp"

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

#include "coolfluid-paths.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace CF {
  namespace Common {

////////////////////////////////////////////////////////////////////////////////

OSystem::OSystem() :
  m_layer(),
  m_lib_loader()
{
  m_layer.reset();
  m_lib_loader.reset();

#ifdef CF_HAVE_DLOPEN
    if ( is_null( m_lib_loader ) )   m_lib_loader.reset( new PosixDlopenLibLoader() );
#endif

#ifdef CF_OS_LINUX
    if ( is_null( m_layer ) ) m_layer.reset( new Linux::OSystemLayer() );
#else
#ifdef CF_OS_MACOSX
    if ( is_null( m_layer ) ) m_layer.reset( new MacOSX::OSystemLayer() );
#else
#ifdef CF_OS_WINDOWS
    if ( is_null( m_layer ) ) m_layer.reset( new Win32::OSystemLayer() );
    if ( is_null( m_lib_loader ) )   m_lib_loader.reset( new Win32::LibLoader() );
#else
  #error "Unkown operating system: not Windows, MacOSX or Linux"
#endif
#endif
#endif

    cf_assert ( is_not_null( m_layer ) );
    cf_assert ( is_not_null( m_lib_loader   ) );
  
  std::vector< boost::filesystem::path > default_paths(1, boost::filesystem::path(CF_BUILD_DIR) / boost::filesystem::path("dso"));
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

  } // Common
} // CF
