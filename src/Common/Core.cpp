// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/tokenizer.hpp>

#include "Common/MPI/PE.hpp"

#include "Common/Log.hpp"
#include "Common/OSystem.hpp"
#include "Common/OSystemLayer.hpp"
#include "Common/NetworkInfo.hpp"
#include "Common/EventHandler.hpp"
#include "Common/OSystem.hpp"
#include "Common/CGroup.hpp"
#include "Common/CLibraries.hpp"
#include "Common/CRoot.hpp"
#include "Common/CEnv.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/BuildInfo.hpp"
#include "Common/CodeProfiler.hpp"
#include "Common/LibLoader.hpp"
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

Core::Core()
{
  // this is most likely the first part of the code to be executed
  // due to library objects registration preceding all other execution.
  // that registration will call Core::instance() to get access to libraries and factories

  // here we ensure that all singletons get created and exist from now on
  // to avoid cyclic creation dependencies
  TypeInfo::instance();
  Logger::instance();
  AssertionManager::instance();
  OSystem::instance().layer()->platform_name();
  mpi::PE::instance();

  // create singleton objects inside core
  m_event_handler.reset ( new EventHandler() );
  m_build_info.reset    ( new BuildInfo()    );
  m_network_info.reset  ( new NetworkInfo()  );

  // create singleton components inside core
  // these are critical to library object registration

  m_environment   = allocate_component<CEnv>( "Environment" );
  m_libraries     = allocate_component<CLibraries>( "Libraries" );
  m_factories     = allocate_component<CFactories>( "Factories" );

  // this types must be registered immedietly on creation,
  // registration could be defered to after the Core has been inialized.
  RegistTypeInfo<CEnv>();
  RegistTypeInfo<CLibraries>();
  RegistTypeInfo<CFactories>();

  // create the root component ant its structure structure
  m_root = CRoot::create("Root");
  m_root->mark_basic();

  // these components are placed on the root structure
  // but ownership is shared with Core, so they get destroyed in ~Core()
  m_root->add_component( m_environment )->mark_basic();
  m_root->add_component( m_libraries )->mark_basic();
  m_root->add_component( m_factories )->mark_basic();

  CGroup::Ptr tools = m_root->create_component<CGroup>("Tools");
  tools->mark_basic();
  tools->properties()["brief"] = std::string("Generic tools");
  tools->properties()["description"] = std::string("");
}

////////////////////////////////////////////////////////////////////////////////

Core::~Core() {}

////////////////////////////////////////////////////////////////////////////////

void Core::initiate ( int argc, char** argv )
{
  m_argc = argc;
  m_argv = argv;
  if ( !mpi::PE::instance().is_init() )
    mpi::PE::instance().init(argc,argv); // this might modify argc and argv
    
  char* env_var = std::getenv("COOLFLUID_PLUGINS");
  if (env_var != NULL) {
    std::string environment_variable_coolfluid_plugins = env_var;
    typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
    boost::char_separator<char> sep(":");
    Tokenizer tokens(environment_variable_coolfluid_plugins, sep);

    for (Tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
        OSystem::instance().lib_loader()->load_library(*tok_iter);
  }
}

////////////////////////////////////////////////////////////////////////////////

void Core::terminate()
{
  if ( mpi::PE::instance().is_init() )
    mpi::PE::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

CRoot::Ptr Core::root() const
{
  cf_assert( is_not_null(m_root) );
  return m_root;
}

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< Common::EventHandler > Core::event_handler() const
{
  cf_assert(m_event_handler != nullptr);
  return m_event_handler;
}

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< Common::BuildInfo > Core::build_info() const
{
  cf_assert(m_build_info != nullptr);
  return m_build_info;
}

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< Common::CEnv > Core::environment() const
{
  cf_assert(m_environment != nullptr);
  return m_environment;
}

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< Common::CLibraries>  Core::libraries() const
{
  cf_assert(m_libraries != nullptr);
  return m_libraries;
}
////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< Common::CFactories > Core::factories() const
{
  cf_assert(m_factories != nullptr);
  return m_factories;
}
////////////////////////////////////////////////////////////////////////////////

void Core::set_profiler(const std::string & builder_name)
{
  CodeProfiler::Ptr profiler =
    create_component_abstract_type<CodeProfiler>(builder_name, "Profiler");
  m_root->add_component( profiler );
}

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr<CodeProfiler> Core::profiler() const
{
  return m_root->get_child_ptr("Profiler")->as_ptr<CodeProfiler>();
}
////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr<NetworkInfo> Core::network_info () const
{
  cf_assert( is_not_null(m_network_info) );
  return m_network_info;
}

////////////////////////////////////////////////////////////////////////////////


} // Common
} // CF
