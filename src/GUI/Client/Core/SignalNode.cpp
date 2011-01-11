// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QString>

#include "Common/CF.hpp"

#include "GUI/Client/Core/SignalNode.hpp"

////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////

SignalNode::SignalNode(const XmlNode * node) :
    m_node(node)
{
  cf_assert(node != nullptr);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString SignalNode::target() const
{
  XmlAttr * attr = m_node->first_attribute("target");

  if(attr != nullptr)
    return attr->value();

  return QString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString SignalNode::sender() const
{
  XmlAttr * attr = m_node->first_attribute("sender");

  if(attr != nullptr)
    return attr->value();

  return QString();

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString SignalNode::receiver() const
{
  XmlAttr * attr = m_node->first_attribute("receiver");

  if(attr != nullptr)
    return attr->value();

  return QString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString SignalNode::type() const
{
  XmlAttr * attr = m_node->first_attribute("type");

  if(attr != nullptr)
    return attr->value();

  return QString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString SignalNode::direction() const
{
  return "???";
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString SignalNode::time() const
{
  XmlAttr * attr = m_node->first_attribute("time");

  if(attr != nullptr)
    return attr->value();

  return QString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

const XmlNode * SignalNode::node() const
{
  return m_node;
}

////////////////////////////////////////////////////////////////////////////

} // namespace ClientCore
} // namespace GUI
} // namespace CF
