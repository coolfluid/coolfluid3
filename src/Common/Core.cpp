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

#include "Common/MPI/PEInterface.hpp"

#include "Common/EventHandler.hpp"
//#include "Common/VarRegistry.hpp"
#include "Common/Log.hpp"
#include "Common/OSystem.hpp"
#include "Common/CGroup.hpp"
#include "Common/CRoot.hpp"
#include "Common/CEnv.hpp"

//#include "Common/SingleBehaviorFactory.hpp"
#include "Common/DirPaths.hpp"
//#include "Common/FileHandlerInput.hpp"
//#include "Common/FileHandlerOutput.hpp"
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
// Common::ConfigObject("CoreEnv"),
//  m_var_registry ( new VarRegistry() )
  m_event_handler(new Common::EventHandler()),
  m_module_registry(new Common::LibraryRegistry()),
  m_factory_registry(new Common::FactoryRegistry())
{
  m_root = CRoot::create("Root");
  m_root->create_component_type<CGroup>("Libraries");
  m_root->create_component_type<CGroup>("Tools");
  m_root->create_component_type<CEnv>("Env");

//  addConfigOptionsTo(this);
//
//  setParameter("DoAssertions",          &(AssertionManager::instance().DoAssertions));
//  setParameter("AssertionDumps",        &(AssertionManager::instance().AssertionDumps));
//  setParameter("AssertionThrows",       &(AssertionManager::instance().AssertionThrows));
//
//  setParameter("ExceptionOutputs",      &(ExceptionManager::instance().ExceptionOutputs));
//  setParameter("ExceptionDumps",        &(ExceptionManager::instance().ExceptionDumps));
//  setParameter("ExceptionAborts",       &(ExceptionManager::instance().ExceptionAborts));
//
//  setParameter("OnlyCPU0Writes",        &(m_env_vars->OnlyCPU0Writes));
//  setParameter("RegistSignalHandlers",  &(m_env_vars->RegistSignalHandlers));
//  setParameter("VerboseEvents",         &(m_env_vars->VerboseEvents));
//  setParameter("ErrorOnUnusedConfig",   &(m_env_vars->ErrorOnUnusedConfig));
//  setParameter("TraceToStdOut",         &(m_env_vars->TraceToStdOut));
//  setParameter("TraceActive",           &(m_env_vars->TraceActive));
//  setParameter("MainLoggerFileName",    &(m_env_vars->MainLoggerFileName));
//  setParameter("ExceptionLogLevel",     &(m_env_vars->ExceptionLogLevel));
}

////////////////////////////////////////////////////////////////////////////////

void Core::setup()
{
  SetupObject::setup();

  // these are the default values
/// @todo port these into k3
//  SingleBehaviorFactory<Common::FileHandlerInput>::instance().setDefaultBehavior("CurlAccessRepository");
//  SingleBehaviorFactory<Common::FileHandlerOutput>::instance().setDefaultBehavior("DirectFileWrite");
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
  ret += "Unknown";
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
  // delete_ptr ( m_var_registry );

  delete m_module_registry;     m_module_registry = CFNULL;
  delete m_factory_registry;    m_factory_registry = CFNULL;

/// @todo should be done like this and these classes probably
///       should be nested and private inside this class
//   delete_ptr(m_LibraryRegistry);
//   delete_ptr(m_factoryRegistry);
}

////////////////////////////////////////////////////////////////////////////////

void Core::initiate ( int argc, char** argv )
{
  PEInterface::instance().init(argc,argv); // this might modify argc and argv
}

////////////////////////////////////////////////////////////////////////////////

void Core::terminate()
{
//  CFinfo << "-------------------------------------------------------------\n" << CFflush;
//  CFinfo << "CF Environment Terminating\n" << CFflush;

//  CFinfo << "Terminating Hook Modules ...\n" << CFflush;

  PEInterface::instance().finalize();

//  CFinfo << "-------------------------------------------------------------\n" << CFflush;
//  CFinfo << "CF Environment Terminated\n";
//  CFinfo << "-------------------------------------------------------------\n" << CFflush;
}

////////////////////////////////////////////////////////////////////////////////

Common::SafePtr<Common::LibraryRegistry> Core::getLibraryRegistry()
{
  cf_assert(m_module_registry != CFNULL);
  return m_module_registry;
}

////////////////////////////////////////////////////////////////////////////////

Common::SafePtr<Common::FactoryRegistry> Core::getFactoryRegistry()
{
  cf_assert(m_factory_registry != CFNULL);
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
