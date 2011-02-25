// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/MPI/PE.hpp"

#include "Common/EventHandler.hpp"
#include "Common/OSystem.hpp"
#include "Common/CGroup.hpp"
#include "Common/CLibraries.hpp"
#include "Common/CRoot.hpp"
#include "Common/CEnv.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/BuildInfo.hpp"
#include "Common/CodeProfiler.hpp"
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
  return m_root;
}

////////////////////////////////////////////////////////////////////////////////

Core::Core() :
  m_event_handler    ( new EventHandler() ),
  m_build_info       ( new BuildInfo() ),
  m_environment      ( new CEnv("Environment") ),
  m_libraries        ( new CLibraries("Libraries") ),
  m_factories        ( new CFactories("Factories") )
{
  // this types must be registered immedietly on creation,
  // registration could be defered to after the Core has been inialized.
  RegistTypeInfo<CEnv>();
  RegistTypeInfo<CLibraries>();
  RegistTypeInfo<CFactories>();

  m_root = CRoot::create("Root");
  m_root->mark_basic();

  m_root->add_component( m_environment )->mark_basic();

  m_root->add_component( m_libraries )->mark_basic();
  m_root->add_component( m_factories )->mark_basic();

  CGroup::Ptr tools = m_root->create_component<CGroup>("Tools");
  tools->mark_basic();
  tools->properties()["brief"] = std::string("General tools");
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
}

////////////////////////////////////////////////////////////////////////////////

void Core::terminate()
{
  if ( mpi::PE::instance().is_init() )
    mpi::PE::instance().finalize();
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
  return m_root->get_child_ptr<CodeProfiler>("Profiler");
}
////////////////////////////////////////////////////////////////////////////////

NetworkInfo& Core::network_info ()
{
  return m_network_info;
}

////////////////////////////////////////////////////////////////////////////////


} // Common
} // CF
