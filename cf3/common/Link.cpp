// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "common/BasicExceptions.hpp"
#include "common/Builder.hpp"
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
  regist_signal( "change_link" )
    .connect( boost::bind( &Link::change_link, this, _1 ) )
    .description("Change link path")
    .pretty_name("Change target");
}


Link::~Link()
{
}


Handle<Component> Link::follow()
{
  return m_link_component;
}

Handle<Component const> Link::follow() const
{
  return m_link_component;
}


bool Link::is_linked () const
{
  return is_not_null(m_link_component);
}

Link& Link::link_to ( Component& lnkto )
{
  if (is_not_null(lnkto.handle<Link>()))
    throw SetupError(FromHere(), "Cannot link a Link to another Link");

  m_link_component = lnkto.handle();
  return *this;
}


void Link::change_link( SignalArgs & args )
{
  SignalOptions options( args );
  SignalFrame reply = args.create_reply();

  std::string path = options.value<std::string>("target_path");
  Handle<Component> target = access_component(path);

  link_to (*target);

  reply.map("options").set_option("target_path", class_name<std::string>(), path);
}

////////////////////////////////////////////////////////////////////////////////

Handle< Component > follow_link(const Handle< Component >& link_or_comp)
{
  Handle<Link> l(link_or_comp);
  if(is_null(l))
    return link_or_comp;

  return l->follow();
}

Handle< const Component > follow_link(const Handle< const Component >& link_or_comp)
{
  Handle<Link const> l(link_or_comp);
  if(is_null(l))
    return link_or_comp;

  return l->follow();
}

Handle< Component > follow_link(Component& link_or_comp)
{
  return follow_link(link_or_comp.handle());
}

Handle< const Component > follow_link(const Component& link_or_comp)
{
  return follow_link(link_or_comp.handle());
}



////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
