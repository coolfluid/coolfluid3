// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/MPI/PE.hpp"

#include "Common/EventHandler.hpp"
#include "Common/OSystem.hpp"
#include "Common/CGroup.hpp"
#include "Common/CRoot.hpp"
#include "Common/CEnv.hpp"
#include "Common/CFactories.hpp"

#include "Common/BuildInfo.hpp"
#include "Common/LibraryRegistry.hpp"
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
  m_build_info(new Common::BuildInfo())
{
  m_root = CRoot::create("Root");

  m_root->create_component_type<CEnv>("Environment");

  m_root->create_component_type<CGroup>("Libraries");

  m_root->create_component_type<CFactories>("Factories")->mark_basic();

  m_root->create_component_type<CGroup>("Tools")->mark_basic();

  m_root->mark_basic(); // root should always be basic
}

////////////////////////////////////////////////////////////////////////////////

Core::~Core() {}

////////////////////////////////////////////////////////////////////////////////

void Core::initiate ( int argc, char** argv )
{
  if ( !PE::instance().is_init() )
    PE::instance().init(argc,argv); // this might modify argc and argv
}

////////////////////////////////////////////////////////////////////////////////

void Core::terminate()
{
  if ( PE::instance().is_init() )
    PE::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

boost::weak_ptr< Common::LibraryRegistry > Core::library_registry()
{
  cf_assert(m_module_registry != nullptr);
  return m_module_registry;
}

////////////////////////////////////////////////////////////////////////////////

boost::weak_ptr< Common::EventHandler > Core::event_handler()
{
  cf_assert(m_event_handler != nullptr);
  return m_event_handler;
}

////////////////////////////////////////////////////////////////////////////////

boost::weak_ptr< Common::BuildInfo > Core::build_info()
{
  cf_assert(m_build_info != nullptr);
  return m_build_info;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
