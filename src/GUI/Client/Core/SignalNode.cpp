// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QString>

#include "rapidxml/rapidxml.hpp"

#include "Common/CF.hpp"
#include "Common/XML/SignalFrame.hpp"

#include "GUI/Client/Core/SignalNode.hpp"

////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////

SignalNode::SignalNode(const SignalFrame * node) :
    m_node(node)
{
  cf_assert(node != nullptr);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString SignalNode::target() const
{
  rapidxml::xml_attribute<>* attr = m_node->node.content->first_attribute("target");

  if(attr != nullptr)
    return attr->value();

  return QString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString SignalNode::sender() const
{
  rapidxml::xml_attribute<>* attr = m_node->node.content->first_attribute("sender");

  if(attr != nullptr)
    return attr->value();

  return QString();

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString SignalNode::receiver() const
{
  rapidxml::xml_attribute<>* attr = m_node->node.content->first_attribute("receiver");

  if(attr != nullptr)
    return attr->value();

  return QString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString SignalNode::type() const
{
  rapidxml::xml_attribute<>* attr = m_node->node.content->first_attribute("type");

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
  rapidxml::xml_attribute<>* attr = m_node->node.content->first_attribute("time");

  if(attr != nullptr)
    return attr->value();

  return QString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

const SignalFrame * SignalNode::node() const
{
  return m_node;
}

////////////////////////////////////////////////////////////////////////////

} // namespace ClientCore
} // namespace GUI
} // namespace CF
