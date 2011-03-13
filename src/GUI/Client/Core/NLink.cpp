// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QAction>
#include <QFileIconProvider>
#include <QMenu>
#include <QModelIndex>

#include "Common/Signal.hpp"
#include "Common/CRoot.hpp"
#include "Common/URI.hpp"
#include "Common/BasicExceptions.hpp"

#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/Core/NTree.hpp"
//#include "GUI/Client/Core/SelectPathDialog.hpp"

#include "GUI/Client/Core/NLink.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::GUI::ClientCore;

NLink::NLink(const QString & name)
  : CNode(name, "CLink", LINK_NODE)
{
  regist_signal("goToTarget", "Switch to the target node", "Go to target node")->
      signal->connect(boost::bind(&NLink::goToTarget, this, _1));

  m_localSignals << "goToTarget";
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NLink::toolTip() const
{
  QString path = "<No target>";

  if(m_target.get() != nullptr)
    path = m_target->full_path().path().c_str();

  return QString("Target: %1").arg(path);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

URI NLink::targetPath() const
{
  if(m_target.get() == nullptr)
    return URI();

  return m_target->full_path();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::setTargetPath(const URI & path)
{
  if(!path.empty())
  {
    CNode::Ptr target = boost::dynamic_pointer_cast<CRoot>(m_root.lock())->retrieve_component<CNode>(path);
    this->setTargetNode(target);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::setTargetNode(const CNode::Ptr & node)
{
  if(node.get() == nullptr)
    NLog::globalLog()->addError("Target is null");
  else
    m_target = node;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::goToTarget(SignalArgs & )
{
	if ( is_null(m_target) )
		throw ValueNotFound (FromHere(), "Target of this link is not set or not valid");

	QModelIndex index = NTree::globalTree()->indexByPath(m_target->full_path());

	if(index.isValid())
		NTree::globalTree()->setCurrentIndex(index);
	else
		NLog::globalLog()->addError(QString("%1: path does not exist")
																.arg(m_target->full_path().path().c_str()));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::change_link(SignalArgs & args)
{
  SignalFrame & options = args.map( Protocol::Tags::key_options() );

  try
  {
    std::string path = options.get_option<std::string>("target_path");

    this->setTargetPath(path);

    NLog::globalLog()->addMessage(QString("Link '%1' now points to '%2'.")
                                     .arg(full_path().path().c_str()).arg(path.c_str()));

  }
  catch(InvalidURI & ip)
  {
    NLog::globalLog()->addError(ip.msg().c_str());
  }
}
