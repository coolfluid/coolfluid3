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
#include "Common/CPath.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
//#include "GUI/Client/Core/SelectPathDialog.hpp"

#include "GUI/Client/Core/NLink.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;

NLink::NLink(const QString & name)
  : CNode(name, "CLink", LINK_NODE)
{
  BUILD_COMPONENT;

  regist_signal("goToTarget", "Switch to the target node", "Go to target node")->connect(boost::bind(&NLink::goToTarget, this, _1));

  m_localSignals << "goToTarget";

//  QAction * action;

//  action = new QAction("Go to target node", m_contextMenu);
//  connect(action, SIGNAL(triggered()), this, SLOT(goToTarget()));
//  m_contextMenu->addAction(action);

//  action = new QAction("Change target path", m_contextMenu);
//  connect(action, SIGNAL(triggered()), this, SLOT(changeTarget()));
//  m_contextMenu->addAction(action);

//  regist_signal("change_link", "Change target")->connect(boost::bind(&NLink::change_link, this, _1));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NLink::getToolTip() const
{
  QString path = "<No target>";

  if(m_target.get() != nullptr)
    path = m_target->full_path().string().c_str();

  return QString("Target: %1").arg(path);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CPath NLink::getTargetPath() const
{
  if(m_target.get() == nullptr)
    return CPath();

  return m_target->full_path();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::setTargetPath(const CPath & path)
{
  CNode::Ptr target = boost::dynamic_pointer_cast<CRoot>(m_root.lock())->access_component<CNode>(path);
  this->setTargetNode(target);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::setTargetNode(const CNode::Ptr & node)
{
  if(node.get() == nullptr)
    ClientRoot::log()->addError("Target is null");
  else
    m_target = node;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::goToTarget(XmlNode & node)
{
  QModelIndex index = ClientRoot::tree()->getIndexByPath(m_target->full_path());

  if(index.isValid())
    ClientRoot::tree()->setCurrentIndex(index);
  else
    ClientRoot::log()->addError(QString("%1: path does not exist")
                                   .arg(m_target->full_path().string().c_str()));
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

//  ClientRoot::core()->sendSignal(*root.get());
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

    ClientRoot::log()->addMessage(QString("Link '%1' now points to '%2'.")
                                     .arg(full_path().string().c_str()).arg(path.c_str()));

  }
  catch(InvalidPath & ip)
  {
    ClientRoot::log()->addError(ip.msg().c_str());
  }
}
