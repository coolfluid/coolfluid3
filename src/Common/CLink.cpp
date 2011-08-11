// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"
#include "Common/CBuilder.hpp"
#include "Common/Signal.hpp"
#include "Common/LibCommon.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Common/CLink.hpp"

using namespace CF::Common::XML;

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CLink, Component, LibCommon > CLink_Builder;

////////////////////////////////////////////////////////////////////////////////

CLink::CLink ( const std::string& name) : Component ( name )
{
  m_is_link = true;

  regist_signal( "change_link" )
    ->connect( boost::bind( &CLink::change_link, this, _1 ) )
    ->description("Change link path")
    ->pretty_name("Change target");
}


CLink::~CLink()
{
}


Component::Ptr CLink::follow()
{
//  cf_assert_desc("Cannot retrieve linked component because it is null", is_not_null(m_link_component.lock()) );
  return m_link_component.lock();
}

Component::ConstPtr CLink::follow() const
{
//  cf_assert_desc("Cannot retrieve linked component because it is null", is_not_null(m_link_component.lock()) );
  return m_link_component.lock();
}


bool CLink::is_linked () const
{
  return !m_link_component.expired();
}


CLink& CLink::link_to ( Component::Ptr lnkto )
{
  if ( is_null(lnkto) )
    throw BadValue(FromHere(), "Cannot link to null component");

  if (lnkto->is_link())
    throw SetupError(FromHere(), "Cannot link a CLink to another CLink");

  m_link_component = lnkto;
  return *this;
}


CLink& CLink::link_to ( Component& lnkto )
{
  if (lnkto.is_link())
    throw SetupError(FromHere(), "Cannot link a CLink to another CLink");

  m_link_component = lnkto.self();
  return *this;
}


CLink& CLink::link_to ( Component const& lnkto )
{
  if (lnkto.is_link())
    throw SetupError(FromHere(), "Cannot link a CLink to another CLink");

  m_link_component = boost::const_pointer_cast<Component>(lnkto.self());
  return *this;
}


void CLink::change_link( SignalArgs & args )
{
  SignalOptions options( args );
  SignalFrame reply = args.create_reply();

  std::string path = options.value<std::string>("target_path");
  Component::Ptr target = m_root.lock()->access_component_ptr(path);

  link_to (target);

  reply.map("options").set_option("target_path", path);
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
