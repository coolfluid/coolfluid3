// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/TreeThread.hpp"
#include "UI/Core/ThreadManager.hpp"

#include "UI/Core/NBrowser.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

NBrowser::NBrowser()
  : CNode(CLIENT_BROWSERS, "NBrowser", CNode::BROWSER_NODE),
    m_counter(0)
{
}

////////////////////////////////////////////////////////////////////////////

QString NBrowser::generateName()
{
 return QString("Browser_%1").arg(m_counter++);
}

////////////////////////////////////////////////////////////////////////////

QString NBrowser::toolTip() const
{
  return this->getComponentType();
}

////////////////////////////////////////////////////////////////////////////

NBrowser::Ptr NBrowser::globalBrowser()
{
  static NBrowser::Ptr browser = ThreadManager::instance().tree().rootChild<NBrowser>(CLIENT_BROWSERS);
  cf_assert( is_not_null(browser.get()) );
  return browser;
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF
