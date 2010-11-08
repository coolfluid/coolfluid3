// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <fstream>

#ifdef CF_HAVE_CONFIG_H
  #include "coolfluid_config.h"
#endif

#include "coolfluid_svnversion.h"

#include "Common/MPI/PE.hpp"

#include "Common/EventHandler.hpp"
#include "Common/Log.hpp"
#include "Common/OSystem.hpp"
#include "Common/CGroup.hpp"
#include "Common/CRoot.hpp"
#include "Common/CEnv.hpp"

#include "Common/DirPaths.hpp"
#include "Common/LibraryRegistry.hpp"
#include "Common/FactoryRegistry.hpp"
#include "Common/LibraryRegisterBase.hpp"

#include "Common/Core.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {

////////////////////////////////////////////////////////////////////////////////

Core& Core::instance()
{
  static Core env;
  return env;
}

////////////////////////////////////////////////////////////////////////////////

CRoot::Ptr Core::root()
{
  return Core::instance().m_root;
}

////////////////////////////////////////////////////////////////////////////////

Core::Core() :
  m_event_handler(new Common::EventHandler()),
  m_module_registry(new Common::LibraryRegistry()),
  m_factory_registry(new Common::FactoryRegistry())
{
  m_root = CRoot::create("Root");

  m_root->create_component_type<CEnv>("Environment");

  m_root->create_component_type<CGroup>("Libraries");

  m_root->create_component_type<CGroup>("Tools")->mark_basic();

  m_root->mark_basic(); // root should always be basic

}

////////////////////////////////////////////////////////////////////////////////

void Core::setup()
{
  SetupObject::setup();
}

////////////////////////////////////////////////////////////////////////////////

void Core::unsetup()
{
  SetupObject::unsetup();
}

////////////////////////////////////////////////////////////////////////////////

std::string Core::getBuildType () const
{
  return CF_BUILD_TYPE;
}

////////////////////////////////////////////////////////////////////////////////

std::string Core::getSvnVersion () const
{
  return CF_SVNVERSION;
}

////////////////////////////////////////////////////////////////////////////////

std::string Core::getReleaseVersion () const
{
  return CF_VERSION_STR;
}

////////////////////////////////////////////////////////////////////////////////

std::string Core::getKernelVersion () const
{
  return CF_KERNEL_VERSION_STR;
}

////////////////////////////////////////////////////////////////////////////////

std::string Core::getBuildProcessor () const
{
  return CF_BUILD_PROCESSOR;
}

////////////////////////////////////////////////////////////////////////////////

std::string Core::getBuildSystem () const
{
  std::string ret;
#ifdef CF_CMAKE_VERSION
  ret += "CMake ";
  ret += CF_CMAKE_VERSION;
#else
  ret += "UNKNOWN";
#endif
  return ret;
}

////////////////////////////////////////////////////////////////////////////////

std::string Core::getSystemName() const
{
  return CF_OS_NAME;
}

////////////////////////////////////////////////////////////////////////////////

std::string Core::getLongSystemName() const
{
  return CF_OS_LONGNAME;
}

////////////////////////////////////////////////////////////////////////////////

std::string Core::getSystemVersion() const
{
  return CF_OS_VERSION;
}

////////////////////////////////////////////////////////////////////////////////

std::string Core::getSystemBits() const
{
  return CF_OS_BITS;
}
////////////////////////////////////////////////////////////////////////////////

std::string Core::getVersionHeader() const
{
  std::ostringstream out;

  out << "Release      : " << getReleaseVersion() << "\n";
  out << "Kernel       : " << getKernelVersion()  << "\n";
  out << "Build System : " << getBuildSystem()    << "\n";
  out << "Build Type   : " << getBuildType()      << "\n";
  out << "Build OS     : " << getLongSystemName() << " [" << getSystemBits() << "bits]\n";
  out << "Build CPU    : " << getBuildProcessor() << "\n";

  return out.str();
}

////////////////////////////////////////////////////////////////////////////////

Core::~Core()
{
  delete m_module_registry;     m_module_registry = nullptr;
  delete m_factory_registry;    m_factory_registry = nullptr;

/// @todo should be done like this and these classes probably
///       should be nested and private inside this class
//   delete_ptr(m_LibraryRegistry);
//   delete_ptr(m_factoryRegistry);
}

////////////////////////////////////////////////////////////////////////////////

void Core::initiate ( int argc, char** argv )
{
  PE::instance().init(argc,argv); // this might modify argc and argv
}

////////////////////////////////////////////////////////////////////////////////

void Core::terminate()
{
//  CFinfo << "Terminating Hook Modules ...\n" << CFflush;
  PE::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

Common::SafePtr<Common::LibraryRegistry> Core::getLibraryRegistry()
{
  cf_assert(m_module_registry != nullptr);
  return m_module_registry;
}

////////////////////////////////////////////////////////////////////////////////

Common::SafePtr<Common::FactoryRegistry> Core::getFactoryRegistry()
{
  cf_assert(m_factory_registry != nullptr);
  return m_factory_registry;
}

////////////////////////////////////////////////////////////////////////////////

Common::SafePtr<Common::EventHandler> Core::getEventHandler()
{
  cf_assert(m_event_handler.isNotNull());
  return m_event_handler.getPtr();
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
