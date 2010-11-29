// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtCore>
#include <QtGui>

#include "GUI/Client/uTests/MyNode.hpp"

using namespace CF::GUI::ClientTest;

MyNode::MyNode(const QString & name)
  : CNode(name, "MyNode", CNode::GENERIC_NODE)
{
  tag_component(this); define_config_properties();
}

////////////////////////////////////////////////////////////////////////////

QIcon MyNode::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::File);
}

////////////////////////////////////////////////////////////////////////////

QString MyNode::toolTip() const
{
  return this->getComponentType();
}

////////////////////////////////////////////////////////////////////////////

