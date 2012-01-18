// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtCore>
#include <QtGui>

#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "test/ui/MyNode.hpp"

using namespace cf3::common;
using namespace cf3::ui::core;
using namespace cf3::ui::CoreTest;

MyNode::MyNode(const std::string & name)
  : CNode(name, "MyNode", CNode::DEBUG_NODE)
{
  options().add_option("theAnswer", 42)
      .description("The answer to the ultimate question of Life, the Universe, and Everything");

  options().add_option("someBool", true)
      .description("The bool value");

  options().add_option("myString", std::string("This is a string") )
      .description("A string");

  properties().add_property("someProp", Real(3.14));

  m_content_listed = true;
}

////////////////////////////////////////////////////////////////////////////

QString MyNode::tool_tip() const
{
  return this->component_type();
}

////////////////////////////////////////////////////////////////////////////

