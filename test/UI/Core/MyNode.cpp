// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtCore>
#include <QtGui>

#include "Common/OptionT.hpp"

#include "test/UI/Core/MyNode.hpp"

using namespace CF::Common;
using namespace CF::UI::Core;
using namespace CF::UI::CoreTest;

MyNode::MyNode(const std::string & name)
  : CNode(name, "MyNode", CNode::DEBUG_NODE)
{
  m_options.add_option< OptionT<int> >("theAnswer", "The answer to the ultimate "
                              "question of Life, the Universe, and Everything", 42);
  m_options.add_option< OptionT<bool> >("someBool", "The bool value", true);

  m_options.add_option< OptionT<std::string> >("myString", "A string", std::string("This is a string") );

  m_properties.add_property("someProp", Real(3.14));

  m_contentListed = true;
}

////////////////////////////////////////////////////////////////////////////

QString MyNode::toolTip() const
{
  return this->componentType();
}

////////////////////////////////////////////////////////////////////////////

