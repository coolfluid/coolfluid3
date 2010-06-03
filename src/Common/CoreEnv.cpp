#include <fstream>

#ifdef CF_HAVE_CONFIG_H
  #include "coolfluid_config.h"
#endif

#include "coolfluid_svnversion.h"

#include "Common/MPI/PEInterface.hpp"

#include "Common/EventHandler.hpp"
#include "Common/VarRegistry.hpp"
#include "Common/Log.hpp"
#include "Common/OSystem.hpp"

//#include "Common/SingleBehaviorFactory.hpp"
#include "Common/DirPaths.hpp"
//#include "Common/FileHandlerInput.hpp"
//#include "Common/FileHandlerOutput.hpp"
#include "Common/ModuleRegistry.hpp"
#include "Common/FactoryRegistry.hpp"
#include "Common/ModuleRegisterBase.hpp"
#include "Common/CoreEnv.hpp"
#include "Common/CoreVars.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {
  namespace Common {

////////////////////////////////////////////////////////////////////////////////

CoreEnv& CoreEnv::getInstance()
{
  static CoreEnv env;
  return env;
}

////////////////////////////////////////////////////////////////////////////////

//void CoreEnv::defineConfigOptions(Common::OptionList& options)
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
  m_event_handler(new Common::EventHandler()),
  m_module_registry(new Common::ModuleRegistry()),
  m_factory_registry(new Common::FactoryRegistry()),
  m_env_vars (new CoreVars()),
  m_var_registry ( new VarRegistry() )
{
//  addConfigOptionsTo(this);
//
//  setParameter("DoAssertions",          &(AssertionManager::getInstance().DoAssertions));
//  setParameter("AssertionDumps",        &(AssertionManager::getInstance().AssertionDumps));
//  setParameter("AssertionThrows",       &(AssertionManager::getInstance().AssertionThrows));
//
//  setParameter("ExceptionOutputs",      &(ExceptionManager::getInstance().ExceptionOutputs));
//  setParameter("ExceptionDumps",        &(ExceptionManager::getInstance().ExceptionDumps));
//  setParameter("ExceptionAborts",       &(ExceptionManager::getInstance().ExceptionAborts));
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
//    OSystem::getInstance().getSignalHandler()->registSignalHandlers();
//    CFinfo << "OK\n" << CFflush;
//  }
//  else
//    CFinfo << "skipping\n" << CFflush;
//
//  CFinfo << "Configuring Logging ... " << CFflush;
//  initLoggers();
//  CFinfo << "OK\n" << CFflush;
//
//  // clean the config.log file
//  boost::filesystem::path fileconfig =
//    Common::DirPaths::getInstance().getResultsDir() / boost::filesystem::path("config.log");
//
////  SelfRegistPtr<Common::FileHandlerOutput> fhandle = Common::SingleBehaviorFactory<Common::FileHandlerOutput>::getInstance().create();
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
//  SingleBehaviorFactory<Common::FileHandlerInput>::getInstance().setDefaultBehavior("CurlAccessRepository");
//  SingleBehaviorFactory<Common::FileHandlerOutput>::getInstance().setDefaultBehavior("DirectFileWrite");
}

////////////////////////////////////////////////////////////////////////////////

void CoreEnv::unsetup()
{
  SetupObject::unsetup();
}

////////////////////////////////////////////////////////////////////////////////

Common::SafePtr<VarRegistry> CoreEnv::getVarRegistry()
{
  return m_var_registry;
}

////////////////////////////////////////////////////////////////////////////////

std::string CoreEnv::getVersionString () const
{
  std::string ret;
  ret += getCFVersion() + " ";
  ret += "Kernel " + getKernelVersion() + " ";
  ret += "( r" + getSvnVersion() + ", " + getBuildType() + " )";
  return ret;
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

std::string CoreEnv::getCFVersion () const
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

CoreEnv::~CoreEnv()
{
  delete_ptr ( m_var_registry );
  delete_ptr ( m_env_vars );

  delete m_module_registry;     m_module_registry = CFNULL;
  delete m_factory_registry;    m_factory_registry = CFNULL;

/// @todo should be done like this and these classes probably
///       should be nested and private inside this class
//   delete_ptr(m_moduleRegistry);
//   delete_ptr(m_factoryRegistry);
}

////////////////////////////////////////////////////////////////////////////////

void CoreEnv::initLoggers()
{
}

////////////////////////////////////////////////////////////////////////////////

void CoreEnv::initiate ( int argc, char** argv )
{
  // Initiate the Parallel environment
  // This modifies argc and argv!
  CFinfo << "-------------------------------------------------------------\n" << CFflush;
  CFinfo << "CF Environment\n" << CFflush;
  CFinfo << "-------------------------------------------------------------\n" << CFflush;

  CFinfo << "Initializing Parallel Environment : " << CFflush;
  PEInterface::getInstance().init(argc,argv);

  CFinfo << "CF version    : " << getVersionString () << "\n";
  CFinfo << "Build system         : " << getBuildSystem() << "\n";
  CFinfo << "Build OS             : " << getLongSystemName() << " [" << getSystemBits() << "bits]\n";
  CFinfo << "Build processor      : " << getBuildProcessor() << "\n";

  m_env_vars->InitArgs.first  = argc;
  m_env_vars->InitArgs.second = argv;

  CFinfo << "Initializing Hook Modules ...\n" << CFflush;
  initiateModules();

  CFinfo << "-------------------------------------------------------------\n" << CFflush;
}

////////////////////////////////////////////////////////////////////////////////

void CoreEnv::initiateModules()
{
  std::vector< SafePtr<ModuleRegisterBase> > mod = m_module_registry->getAllModules();
  std::for_each(mod.begin(),
                mod.end(),
                Common::safeptr_mem_fun(&ModuleRegisterBase::initiate));
}

////////////////////////////////////////////////////////////////////////////////

void CoreEnv::terminateModules()
{
  std::vector< SafePtr<ModuleRegisterBase> > mod = m_module_registry->getAllModules();
  std::for_each(mod.begin(),
                mod.end(),
                Common::safeptr_mem_fun(&ModuleRegisterBase::terminate));
}

////////////////////////////////////////////////////////////////////////////////

void CoreEnv::terminate()
{
  CFinfo << "-------------------------------------------------------------\n" << CFflush;
  CFinfo << "CF Environment Terminating\n" << CFflush;

  CFinfo << "Terminating Hook Modules ...\n" << CFflush;
  terminateModules();

  PEInterface::getInstance().finalize();

  CFinfo << "-------------------------------------------------------------\n" << CFflush;
  CFinfo << "CF Environment Terminated\n";
  CFinfo << "-------------------------------------------------------------\n" << CFflush;
}

////////////////////////////////////////////////////////////////////////////////

Common::SafePtr<Common::CoreVars> CoreEnv::getVars()
{
  cf_assert(m_env_vars != CFNULL);
  return m_env_vars;
}

////////////////////////////////////////////////////////////////////////////////

Common::SafePtr<Common::ModuleRegistry> CoreEnv::getModuleRegistry()
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

  } // namespace Common

} // namespace CF
