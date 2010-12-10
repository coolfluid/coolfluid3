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
  m_properties.add_option< CF::Common::OptionT<int> >("theAnswer", "The answer to the ultimate "
                              "question of Life, the Universe, and Everything", 42);
  m_properties.add_option< CF::Common::OptionT<bool> >("someBool", "The bool value", true);
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

