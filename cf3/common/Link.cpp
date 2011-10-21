// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/BasicExceptions.hpp"
#include "common/CBuilder.hpp"
#include "common/Signal.hpp"
#include "common/LibCommon.hpp"

#include "common/XML/SignalOptions.hpp"

#include "common/Link.hpp"

using namespace cf3::common::XML;

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Link, Component, LibCommon > Link_Builder;

////////////////////////////////////////////////////////////////////////////////

Link::Link ( const std::string& name) : Component ( name )
{
  m_is_link = true;

  regist_signal( "change_link" )
    ->connect( boost::bind( &Link::change_link, this, _1 ) )
    ->description("Change link path")
    ->pretty_name("Change target");
}


Link::~Link()
{
}


Component::Ptr Link::follow()
{
//  cf3_assert_desc("Cannot retrieve linked component because it is null", is_not_null(m_link_component.lock()) );
  return m_link_component.lock();
}

Component::ConstPtr Link::follow() const
{
//  cf3_assert_desc("Cannot retrieve linked component because it is null", is_not_null(m_link_component.lock()) );
  return m_link_component.lock();
}


bool Link::is_linked () const
{
  return !m_link_component.expired();
}


Link& Link::link_to ( Component::Ptr lnkto )
{
  if ( is_null(lnkto) )
    throw BadValue(FromHere(), "Cannot link to null component");

  if (lnkto->is_link())
    throw SetupError(FromHere(), "Cannot link a Link to another Link");

  m_link_component = lnkto;
  return *this;
}


Link& Link::link_to ( Component& lnkto )
{
  if (lnkto.is_link())
    throw SetupError(FromHere(), "Cannot link a Link to another Link");

  m_link_component = lnkto.self();
  return *this;
}


Link& Link::link_to ( Component const& lnkto )
{
  if (lnkto.is_link())
    throw SetupError(FromHere(), "Cannot link a Link to another Link");

  m_link_component = boost::const_pointer_cast<Component>(lnkto.self());
  return *this;
}


void Link::change_link( SignalArgs & args )
{
  SignalOptions options( args );
  SignalFrame reply = args.create_reply();

  std::string path = options.value<std::string>("target_path");
  Component::Ptr target = m_root.lock()->access_component_ptr(path);

  link_to (target);

  reply.map("options").set_option("target_path", path);
}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
