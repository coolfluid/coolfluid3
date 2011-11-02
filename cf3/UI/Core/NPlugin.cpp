// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "UI/Core/NPlugin.hpp"
#include "common/XML/SignalOptions.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

////////////////////////////////////////////////////////////////////////////

NPlugin::NPlugin(const std::string & name)
  : CNode(name.c_str(), "NPlugin", CNode::LOCAL_NODE)
{

}

////////////////////////////////////////////////////////////////////////////

NPlugin::~NPlugin()
{

}

////////////////////////////////////////////////////////////////////////////

SignalPtr NPlugin::add_signal( const std::string& name,
                              const std::string& descr,
                              const std::string& readable_name )
{
  SignalPtr signal = regist_signal(name)
      ->description(descr)
      ->pretty_name(readable_name);

  m_local_signals << name.c_str();

  return signal;
}

////////////////////////////////////////////////////////////////////////////

QString NPlugin::tool_tip() const
{
  return QString("%1 plugin").arg( name().c_str() );
}

////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3
