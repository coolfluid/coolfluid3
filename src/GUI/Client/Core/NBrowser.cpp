// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFileIconProvider>

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/NBrowser.hpp"

using namespace CF::GUI::ClientCore;

NBrowser::NBrowser()
  : CNode(CLIENT_BROWSERS, "NBrowser", CNode::BROWSER_NODE),
    m_counter(0)
{
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NBrowser::generateName()
{
 return QString("Browser_%1").arg(m_counter++);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NBrowser::toolTip() const
{
  return this->getComponentType();
}

