// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/PropertyList.hpp"

#include "ui/core/ThreadManager.hpp"
#include "ui/core/TreeThread.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include "ui/core/NPlugins.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

////////////////////////////////////////////////////////////////////////////

NPlugins::NPlugins(const std::string & name)
  : CNode(name, "NPlugins", CNode::LOCAL_NODE)
{
  properties()["brief"] = std::string("Manages the client plugins.");
  properties()["description"] =
      std::string("All the plugins currently loaded are referenced as "
                  "children of this component. Each allows to do specific "
                  "actions defined by the related plugin.");
}

////////////////////////////////////////////////////////////////////////////

NPlugins::~NPlugins()
{

}

////////////////////////////////////////////////////////////////////////////

QString NPlugins::tool_tip() const
{
  return "Client plugin manager.";
}

////////////////////////////////////////////////////////////////////////////

Handle< NPlugins > NPlugins::global()
{
  static Handle< NPlugins > plugins =
      ThreadManager::instance().tree().root_child<NPlugins>(CLIENT_PLUGINS);
  cf3_assert( is_not_null(plugins.get()) );
  return plugins;
}

////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3
