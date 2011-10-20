// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QModelIndex>

#include "common/CRoot.hpp"
#include "common/Signal.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "UI/Core/NLog.hpp"
#include "UI/Core/NTree.hpp"

#include "UI/Core/NLink.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::UI::Core;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

NLink::NLink(const std::string & name)
  : CNode(name, "CLink", CNode::STANDARD_NODE)
{
  regist_signal( "goToTarget" )
    ->connect( boost::bind( &NLink::go_to_target, this, _1 ) )
    ->description("Switch to the target node")
    ->pretty_name("Go to target node");

  m_local_signals << "goToTarget";
}

//////////////////////////////////////////////////////////////////////////////

QString NLink::tool_tip() const
{
  QString path = "<No target>";

  if(m_target.get() != nullptr)
    path = m_target->uri().path().c_str();

  return QString("Target: %1").arg(path);
}

//////////////////////////////////////////////////////////////////////////////

URI NLink::target_path() const
{
  if(m_target.get() == nullptr)
    return URI();

  return m_target->uri();
}

//////////////////////////////////////////////////////////////////////////////

void NLink::set_target_path(const URI & path)
{
  cf3_assert( !m_root.expired() );

  if(!path.empty())
  {
    CNode::Ptr target = boost::dynamic_pointer_cast<CRoot>(m_root.lock())->retrieve_component<CNode>(path);
    cf3_assert( is_not_null(target.get()) );
    this->set_target_node(target);
  }
}

//////////////////////////////////////////////////////////////////////////////

void NLink::set_target_node(const CNode::Ptr & node)
{
  cf3_assert( is_not_null(node.get()) );

  m_target = node;
}

//////////////////////////////////////////////////////////////////////////////

void NLink::go_to_target(SignalArgs & )
{
  if ( is_null(m_target) )
    throw ValueNotFound (FromHere(), "Target of this link is not set or not valid");

  QModelIndex index = NTree::global()->index_from_path(m_target->uri());

  if(index.isValid())
    NTree::global()->set_current_index(index);
  else
    throw ValueNotFound (FromHere(), m_target->uri().string() + ": path does not exist");
}

//////////////////////////////////////////////////////////////////////////////

void NLink::change_link(SignalArgs & args)
{
  SignalOptions options( args );

  try
  {
    std::string path = options.value<std::string>("target_path");

    this->set_target_path(path);

    NLog::global()->add_message(QString("Link '%1' now points to '%2'.")
                                     .arg(uri().path().c_str()).arg(path.c_str()));

  }
  catch(InvalidURI & ip)
  {
    NLog::global()->add_error(ip.msg().c_str());
  }
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3
