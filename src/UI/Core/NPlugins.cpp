// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "UI/Core/ThreadManager.hpp"
#include "UI/Core/TreeThread.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/NPlugins.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

NPlugins::NPlugins(const std::string & name)
  : CNode(name, "NPlugins", CNode::LOCAL_NODE)
{
  m_properties["brief"] = std::string("Manages the client plugins.");
  m_properties["description"] =
      std::string("All the plugins currently loaded are references as "
                  "children of this component. Each allows to do specific "
                  "actions defined by the related plugin.");
}

////////////////////////////////////////////////////////////////////////////

NPlugins::~NPlugins()
{

}

////////////////////////////////////////////////////////////////////////////

QString NPlugins::toolTip() const
{
  return "Client plugin manager.";
}

////////////////////////////////////////////////////////////////////////////

NPlugins::Ptr NPlugins::globalPlugins()
{
  static NPlugins::Ptr plugins =
      ThreadManager::instance().tree().rootChild<NPlugins>(CLIENT_PLUGINS);
  cf_assert( is_not_null(plugins.get()) );
  return plugins;
}

////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF
