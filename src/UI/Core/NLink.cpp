// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QModelIndex>

#include "Common/CRoot.hpp"
#include "Common/Signal.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "UI/Core/NLog.hpp"
#include "UI/Core/NTree.hpp"

#include "UI/Core/NLink.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::UI::Core;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

NLink::NLink(const std::string & name)
  : CNode(name, "CLink", CNode::STANDARD_NODE)
{
  regist_signal( "goToTarget" )
    ->connect( boost::bind( &NLink::goToTarget, this, _1 ) )
    ->description("Switch to the target node")
    ->pretty_name("Go to target node");

  m_localSignals << "goToTarget";
}

//////////////////////////////////////////////////////////////////////////////

QString NLink::toolTip() const
{
  QString path = "<No target>";

  if(m_target.get() != nullptr)
    path = m_target->uri().path().c_str();

  return QString("Target: %1").arg(path);
}

//////////////////////////////////////////////////////////////////////////////

URI NLink::targetPath() const
{
  if(m_target.get() == nullptr)
    return URI();

  return m_target->uri();
}

//////////////////////////////////////////////////////////////////////////////

void NLink::setTargetPath(const URI & path)
{
  cf_assert( !m_root.expired() );

  if(!path.empty())
  {
    CNode::Ptr target = boost::dynamic_pointer_cast<CRoot>(m_root.lock())->retrieve_component<CNode>(path);
    cf_assert( is_not_null(target.get()) );
    this->setTargetNode(target);
  }
}

//////////////////////////////////////////////////////////////////////////////

void NLink::setTargetNode(const CNode::Ptr & node)
{
  cf_assert( is_not_null(node.get()) );

  m_target = node;
}

//////////////////////////////////////////////////////////////////////////////

void NLink::goToTarget(SignalArgs & )
{
  if ( is_null(m_target) )
    throw ValueNotFound (FromHere(), "Target of this link is not set or not valid");

  QModelIndex index = NTree::globalTree()->indexFromPath(m_target->uri());

  if(index.isValid())
    NTree::globalTree()->setCurrentIndex(index);
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

    this->setTargetPath(path);

    NLog::globalLog()->addMessage(QString("Link '%1' now points to '%2'.")
                                     .arg(uri().path().c_str()).arg(path.c_str()));

  }
  catch(InvalidURI & ip)
  {
    NLog::globalLog()->addError(ip.msg().c_str());
  }
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF
