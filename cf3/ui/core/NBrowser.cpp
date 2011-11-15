// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "ui/uicommon/ComponentNames.hpp"

#include "ui/core/TreeThread.hpp"
#include "ui/core/ThreadManager.hpp"

#include "ui/core/NBrowser.hpp"

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

std::string NBrowser::generate_name()
{
 return QString("Browser_%1").arg(m_counter++).toStdString();
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
} // ui
} // cf3
