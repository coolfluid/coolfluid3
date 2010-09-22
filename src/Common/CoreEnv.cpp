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

//#include "Common/SingleBehaviorFactory.hpp"
#include "Common/DirPaths.hpp"
//#include "Common/FileHandlerInput.hpp"
//#include "Common/FileHandlerOutput.hpp"
#include "Common/LibraryRegistry.hpp"
#include "Common/FactoryRegistry.hpp"
#include "Common/LibraryRegisterBase.hpp"
#include "Common/CoreEnv.hpp"
#include "Common/CoreVars.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {
  namespace Common {

////////////////////////////////////////////////////////////////////////////////

CoreEnv& CoreEnv::instance()
{
  static CoreEnv env;
  return env;
}

////////////////////////////////////////////////////////////////////////////////

//void CoreEnv::defineConfigProperties(Common::OptionList& options)
//{
//  options.addConfigOption< bool >    ("OnlyCPU0Writes",    "Only CPU0 writes to stdout");
//  options.addConfigOption< bool >    ("DoAssertions",      "Turn off assertions dynamically");
//  options.addConfigOption< bool >    ("AssertionDumps",    "If assertions should dump backtraces");
//  options.addConfigOption< bool >    ("AssertionThrows",   "If assertions should throw exceptions instead of aborting code");
//  options.addConfigOption< bool >    ("ExceptionOutputs",  "If exception contructor should output");
//  options.addConfigOption< bool >    ("ExceptionDumps",    "If exception contructor should dump backtrace");
//  options.addConfigOption< bool >    ("ExceptionAborts",   "If exception contructor should abort execution immedietly");
//  options.addConfigOption< Uint >    ("ExceptionLogLevel", "Loglevel for exceptions");
//  options.addConfigOption< bool >    ("RegistSignalHandlers", "If CPU signal handlers should be registered");
//  options.addConfigOption< bool >    ("TraceToStdOut",     "If Tracing should be sent to stdout also");
//  options.addConfigOption< bool >    ("TraceActive",       "If Tracing should be active");
//  options.addConfigOption< bool >    ("VerboseEvents",     "If Events have verbose output");
//  options.addConfigOption< bool >    ("ErrorOnUnusedConfig","Signal error when some user provided config parameters are not used");
//  options.addConfigOption< std::string >("MainLoggerFileName", "Name of main log file");
//}

////////////////////////////////////////////////////////////////////////////////

CoreEnv::CoreEnv() :
// Common::ConfigObject("CoreEnv"),
//  m_var_registry ( new VarRegistry() )
  m_event_handler(new Common::EventHandler()),
  m_module_registry(new Common::LibraryRegistry()),
  m_factory_registry(new Common::FactoryRegistry()),
  m_env_vars (new CoreVars())
{
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

//void CoreEnv::configure ( Common::ConfigArgs& args )
//{
//  ConfigObject::configure(args);
//
//  CFinfo << "Configuring OSystem signal handlers ... " << CFflush;
//  if ( m_env_vars->RegistSignalHandlers )
//  {
//    OSystem::instance().getSignalHandler()->registSignalHandlers();
//    CFinfo << "OK\n" << CFflush;
//  }
//  else
//    CFinfo << "skipping\n" << CFflush;
//
//  CFinfo << "Configuring Logging ... " << CFflush;
//  CFinfo << "OK\n" << CFflush;
//
//  // clean the config.log file
//  boost::filesystem::path fileconfig =
//    Common::DirPaths::instance().getResultsDir() / boost::filesystem::path("config.log");
//
////  SelfRegistPtr<Common::FileHandlerOutput> fhandle = Common::SingleBehaviorFactory<Common::FileHandlerOutput>::instance().create();
////  std::ofstream& fout = fhandle->open(fileconfig,std::ios_base::trunc);
////
////  fout << "CONFIG LOG:" << "\n";
////  fhandle->close();
//}

////////////////////////////////////////////////////////////////////////////////

void CoreEnv::setup()
{
  SetupObject::setup();

  // these are the default values
/// @todo port these into k3
//  SingleBehaviorFactory<Common::FileHandlerInput>::instance().setDefaultBehavior("CurlAccessRepository");
//  SingleBehaviorFactory<Common::FileHandlerOutput>::instance().setDefaultBehavior("DirectFileWrite");
}

////////////////////////////////////////////////////////////////////////////////

void CoreEnv::unsetup()
{
  SetupObject::unsetup();
}

////////////////////////////////////////////////////////////////////////////////

std::string CoreEnv::getBuildType () const
{
  return CF_BUILD_TYPE;
}

////////////////////////////////////////////////////////////////////////////////

std::string CoreEnv::getSvnVersion () const
{
  return CF_SVNVERSION;
}

////////////////////////////////////////////////////////////////////////////////

std::string CoreEnv::getReleaseVersion () const
{
  return CF_VERSION_STR;
}

////////////////////////////////////////////////////////////////////////////////

std::string CoreEnv::getKernelVersion () const
{
  return CF_KERNEL_VERSION_STR;
}

////////////////////////////////////////////////////////////////////////////////

std::string CoreEnv::getBuildProcessor () const
{
  return CF_BUILD_PROCESSOR;
}

////////////////////////////////////////////////////////////////////////////////

std::string CoreEnv::getBuildSystem () const
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

std::string CoreEnv::getSystemName() const
{
  return CF_OS_NAME;
}

////////////////////////////////////////////////////////////////////////////////

std::string CoreEnv::getLongSystemName() const
{
  return CF_OS_LONGNAME;
}

////////////////////////////////////////////////////////////////////////////////

std::string CoreEnv::getSystemVersion() const
{
  return CF_OS_VERSION;
}

////////////////////////////////////////////////////////////////////////////////

std::string CoreEnv::getSystemBits() const
{
  return CF_OS_BITS;
}
////////////////////////////////////////////////////////////////////////////////

std::string CoreEnv::getVersionHeader() const
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

CoreEnv::~CoreEnv()
{
  // delete_ptr ( m_var_registry );
  delete_ptr ( m_env_vars );

  delete m_module_registry;     m_module_registry = CFNULL;
  delete m_factory_registry;    m_factory_registry = CFNULL;

/// @todo should be done like this and these classes probably
///       should be nested and private inside this class
//   delete_ptr(m_LibraryRegistry);
//   delete_ptr(m_factoryRegistry);
}

////////////////////////////////////////////////////////////////////////////////

void CoreEnv::initiate ( int argc, char** argv )
{
//  CFinfo << "-------------------------------------------------------------\n" << CFflush;
//  CFinfo << "CF Core Environment\n" << CFflush;
//  CFinfo << "-------------------------------------------------------------\n" << CFflush;

//  CFinfo << getVersionHeader() << CFflush;

//  CFinfo << "-------------------------------------------------------------\n" << CFflush;

//  CFinfo << "Initializing Parallel Environment ..." << CFendl;

  PEInterface::instance().init(argc,argv); // this might modify argc and argv

  m_env_vars->InitArgs.first  = argc;
  m_env_vars->InitArgs.second = argv;

//  CFinfo << "Initializing Hook Modules ..." << CFendl;

  initiate_modules();

//  CFinfo << "-------------------------------------------------------------\n" << CFflush;
}

////////////////////////////////////////////////////////////////////////////////

void CoreEnv::initiate_modules()
{
  std::vector< SafePtr<LibraryRegisterBase> > mod = m_module_registry->getAllModules();
  std::for_each(mod.begin(),
                mod.end(),
                Common::safeptr_mem_fun(&LibraryRegisterBase::initiate));
}

////////////////////////////////////////////////////////////////////////////////

void CoreEnv::terminate_modules()
{
  std::vector< SafePtr<LibraryRegisterBase> > mod = m_module_registry->getAllModules();
  std::for_each(mod.begin(),
                mod.end(),
                Common::safeptr_mem_fun(&LibraryRegisterBase::terminate));
}

////////////////////////////////////////////////////////////////////////////////

void CoreEnv::terminate()
{
//  CFinfo << "-------------------------------------------------------------\n" << CFflush;
//  CFinfo << "CF Environment Terminating\n" << CFflush;

//  CFinfo << "Terminating Hook Modules ...\n" << CFflush;
  terminate_modules();

  PEInterface::instance().finalize();

//  CFinfo << "-------------------------------------------------------------\n" << CFflush;
//  CFinfo << "CF Environment Terminated\n";
//  CFinfo << "-------------------------------------------------------------\n" << CFflush;
}

////////////////////////////////////////////////////////////////////////////////

Common::SafePtr<Common::CoreVars> CoreEnv::getVars()
{
  cf_assert(m_env_vars != CFNULL);
  return m_env_vars;
}

////////////////////////////////////////////////////////////////////////////////

Common::SafePtr<Common::LibraryRegistry> CoreEnv::getLibraryRegistry()
{
  cf_assert(m_module_registry != CFNULL);
  return m_module_registry;
}

////////////////////////////////////////////////////////////////////////////////

Common::SafePtr<Common::FactoryRegistry> CoreEnv::getFactoryRegistry()
{
  cf_assert(m_factory_registry != CFNULL);
  return m_factory_registry;
}

////////////////////////////////////////////////////////////////////////////////

Common::SafePtr<Common::EventHandler> CoreEnv::getEventHandler()
{
  cf_assert(m_event_handler.isNotNull());
  return m_event_handler.getPtr();
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
