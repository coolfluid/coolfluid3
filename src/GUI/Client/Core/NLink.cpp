// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QAction>
#include <QFileIconProvider>
#include <QMenu>
#include <QModelIndex>

#include "Common/CF.hpp"
#include "Common/CRoot.hpp"
#include "Common/URI.hpp"
#include "Common/BasicExceptions.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
//#include "GUI/Client/Core/SelectPathDialog.hpp"

#include "GUI/Client/Core/NLink.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;

NLink::NLink(const QString & name)
  : CNode(name, "CLink", LINK_NODE)
{
  regist_signal("goToTarget", "Switch to the target node", "Go to target node")->connect(boost::bind(&NLink::goToTarget, this, _1));

  m_localSignals << "goToTarget";
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NLink::toolTip() const
{
  QString path = "<No target>";

  if(m_target.get() != nullptr)
    path = m_target->full_path().string_without_scheme().c_str();

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
    CNode::Ptr target = boost::dynamic_pointer_cast<CRoot>(m_root.lock())->access_component<CNode>(path);
    this->setTargetNode(target);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::setTargetNode(const CNode::Ptr & node)
{
  if(node.get() == nullptr)
    ClientRoot::instance().log()->addError("Target is null");
  else
    m_target = node;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::goToTarget(XmlNode & node)
{
	if ( is_null(m_target) )
		throw ValueNotFound (FromHere(), "Target of this link is not set or not valid");

	QModelIndex index = ClientRoot::instance().tree()->indexByPath(m_target->full_path());

	if(index.isValid())
		ClientRoot::instance().tree()->setCurrentIndex(index);
	else
		ClientRoot::instance().log()->addError(QString("%1: path does not exist")
																.arg(m_target->full_path().string_without_scheme().c_str()));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::changeTarget()
{
	throw NotImplemented(FromHere(), "This feature uses GUI");
//  SelectPathDialog spd;

//  CPath path = spd.show(m_target->full_path());

//  boost::shared_ptr<XmlDoc> root = XmlOps::create_doc();
//  XmlNode * docNode = XmlOps::goto_doc_node(*root.get());

//  XmlNode * signal = XmlOps::add_signal_frame(*docNode, "change_link", full_path(),
//                           full_path(), true);
//  XmlParams p(*signal);

//  p.add_option("target_path", path.string());

//  ClientRoot::instance().core()->sendSignal(*root.get());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::change_link(CF::Common::XmlNode & node)
{
  XmlParams p(node);

  try
  {
    std::string path = p.get_option<std::string>("target_path");

    this->setTargetPath(p.get_option<std::string>("target_path"));

    ClientRoot::instance().log()->addMessage(QString("Link '%1' now points to '%2'.")
                                     .arg(full_path().string_without_scheme().c_str()).arg(path.c_str()));

  }
  catch(InvalidURI & ip)
  {
    ClientRoot::instance().log()->addError(ip.msg().c_str());
  }
}
