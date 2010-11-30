// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"
#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"
#include "Common/XmlHelpers.hpp"

#include "Common/CLink.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CLink, Component, LibCommon > CLink_Builder;

////////////////////////////////////////////////////////////////////////////////

CLink::CLink ( const std::string& name) : Component ( name )
{
    define_config_properties();
  m_is_link = true;

  regist_signal("change_link", "Change link path", "Change target")->connect(boost::bind(&CLink::change_link, this, _1));
}

////////////////////////////////////////////////////////////////////////////////

CLink::~CLink()
{
}

////////////////////////////////////////////////////////////////////////////////

Component::Ptr CLink::get ()
{
  return m_link_component.lock();
}

Component::ConstPtr CLink::get () const
{
  return m_link_component.lock();
}

////////////////////////////////////////////////////////////////////////////////

void CLink::link_to ( Component::Ptr lnkto )
{
  if (lnkto->is_link())
    throw SetupError(FromHere(), "Cannot link a CLink to another CLink");

  m_link_component = lnkto;
}

////////////////////////////////////////////////////////////////////////////////

void CLink::change_link( XmlNode & node )
{
  XmlParams p(node);

  std::string path = p.get_option<std::string>("target_path");
  Component::Ptr target = m_root.lock()->look_component(path);

  link_to (target);

  XmlNode * reply = XmlOps::add_reply_frame(node);

  XmlParams(*reply).add_option("target_path", path);
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
