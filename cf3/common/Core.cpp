// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/tokenizer.hpp>

#include "common/PE/Comm.hpp"

#include "common/Log.hpp"
#include "common/LibCommon.hpp"
#include "common/Signal.hpp"
#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"
#include "common/NetworkInfo.hpp"
#include "common/EventHandler.hpp"
#include "common/OSystem.hpp"
#include "common/Group.hpp"
#include "common/Libraries.hpp"
#include "common/Factories.hpp"
#include "common/Environment.hpp"
#include "common/PropertyList.hpp"

#include "common/BuildInfo.hpp"
#include "common/CodeProfiler.hpp"
#include "common/LibLoader.hpp"
#include "common/Core.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

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
  PE::Comm::instance();
  EventHandler::instance();

  // create singleton objects inside core
  m_build_info.reset    ( new BuildInfo()    );
  m_network_info.reset  ( new NetworkInfo()  );


  // this types must be registered immediately on creation,
  // registration could be defered to after the Core has been inialized.
  RegistTypeInfo<Environment,LibCommon>();
  RegistTypeInfo<Libraries,LibCommon>();
  RegistTypeInfo<Factories,LibCommon>();

  // create the root component and its structure structure
  m_environment = allocate_component<Environment>( "Environment" );
  m_libraries   = allocate_component<Libraries>("Libraries");
  m_factories   = allocate_component<Factories>("Factories");

  m_root = allocate_component<Group>( "Root" );
  m_root->mark_basic();
  m_root->add_component(m_environment);
  m_root->add_component(m_libraries);
  m_root->add_component(m_factories);

  // create tools
  Handle<Group> tools = m_root->create_component<Group>("Tools");
  tools->mark_basic();
  tools->properties()["brief"] = std::string("Generic tools");
  tools->properties()["description"] = std::string("");

}

Core::~Core()
{
  // Make sure libs are terminated before the destruction of root
  terminate();
}

////////////////////////////////////////////////////////////////////////////////

void Core::initiate ( int argc, char** argv )
{
  m_argc = argc;
  m_argv = argv;

  if( environment().options().value<bool>("regist_signal_handlers") )
    OSystem::instance().layer()->regist_os_signal_handlers();

  // initiate the logging facility
  Logger::instance().initiate();

  // load libraries listed in the COOLFLUID_PLUGINS environment variable

  char* env_var = std::getenv("COOLFLUID_PLUGINS");
  if (env_var != NULL)
  {
    std::string environment_variable_coolfluid_plugins = env_var;
    typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
    boost::char_separator<char> sep(":");
    Tokenizer tokens(environment_variable_coolfluid_plugins, sep);

    for (Tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
        OSystem::instance().lib_loader()->load_library(*tok_iter);
  }

  // initiate here all the libraries which the kernel was linked to

  libraries().initiate_all_libraries();

}

////////////////////////////////////////////////////////////////////////////////

void Core::terminate()
{
  // terminate all
  if(is_not_null(m_libraries))
    libraries().terminate_all_libraries();

  m_root.reset();
  m_environment.reset();
  m_libraries.reset();
  m_factories.reset();
}

////////////////////////////////////////////////////////////////////////////////

Component& Core::root() const
{
  cf3_assert( is_not_null(m_root) );
  return *m_root;
}

////////////////////////////////////////////////////////////////////////////////

common::EventHandler& Core::event_handler() const
{
  return EventHandler::instance();
}

////////////////////////////////////////////////////////////////////////////////

common::BuildInfo& Core::build_info() const
{
  cf3_assert(m_build_info != nullptr);
  return *m_build_info;
}

////////////////////////////////////////////////////////////////////////////////

common::Environment& Core::environment() const
{
  cf3_assert(m_environment != nullptr);
  return *m_environment;
}

////////////////////////////////////////////////////////////////////////////////

common::Libraries&  Core::libraries() const
{
  cf3_assert(is_not_null(m_libraries));
  return *m_libraries;
}

////////////////////////////////////////////////////////////////////////////////

common::Factories& Core::factories() const
{
  cf3_assert(is_not_null(m_factories));
  return *m_factories;
}

////////////////////////////////////////////////////////////////////////////////

common::Group& Core::tools() const
{
  Handle<Group> t(root().get_child("Tools"));
  cf3_assert(is_not_null(t));
  return *t;
}

////////////////////////////////////////////////////////////////////////////////

void Core::set_profiler(const std::string & builder_name)
{
  boost::shared_ptr<CodeProfiler> profiler =
    build_component_abstract_type<CodeProfiler>(builder_name, "Profiler");
  m_root->add_component( profiler );
}

////////////////////////////////////////////////////////////////////////////////

Handle<CodeProfiler> Core::profiler() const
{
  return Handle<CodeProfiler>(m_root->get_child("Profiler"));
}
////////////////////////////////////////////////////////////////////////////////

NetworkInfo& Core::network_info () const
{
  cf3_assert( is_not_null(m_network_info) );
  return *m_network_info;
}

////////////////////////////////////////////////////////////////////////////////


} // common
} // cf3
