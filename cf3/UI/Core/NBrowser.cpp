// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/TreeThread.hpp"
#include "UI/Core/ThreadManager.hpp"

#include "UI/Core/NBrowser.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

////////////////////////////////////////////////////////////////////////////

NBrowser::NBrowser()
  : CNode(CLIENT_BROWSERS, "NBrowser", CNode::DEBUG_NODE),
    m_counter(0)
{
}

////////////////////////////////////////////////////////////////////////////

QString NBrowser::generate_name()
{
 return QString("Browser_%1").arg(m_counter++);
}

////////////////////////////////////////////////////////////////////////////

QString NBrowser::tool_tip() const
{
  return this->component_type();
}

////////////////////////////////////////////////////////////////////////////

NBrowser::Ptr NBrowser::global()
{
  static NBrowser::Ptr browser = ThreadManager::instance().tree().root_child<NBrowser>(CLIENT_BROWSERS);
  cf3_assert( is_not_null(browser.get()) );
  return browser;
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3
