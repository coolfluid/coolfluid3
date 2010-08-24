#include <QtCore>
#include <QtGui>

#include "Common/CF.hpp"
#include "Common/CPath.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/SelectPathDialog.hpp"

#include "GUI/Client/NLink.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

NLink::NLink(const QString & name)
  : CNode(name, "CLink", LINK_NODE)
{
  BUILD_COMPONENT;

  QAction * action;

  action = new QAction("Go to target node", m_contextMenu);
  connect(action, SIGNAL(triggered()), this, SLOT(goToTarget()));
  m_contextMenu->addAction(action);

  action = new QAction("Change target path", m_contextMenu);
  connect(action, SIGNAL(triggered()), this, SLOT(changeTarget()));
  m_contextMenu->addAction(action);

  regist_signal("change_link", "Change target")->connect(boost::bind(&NLink::change_link, this, _1));

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NLink::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::Network);
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NLink::getToolTip() const
{
  QString path = "<No target>";

  if(m_target.get() != CFNULL)
    path = m_target->full_path().string().c_str();

  return QString("Target: %1").arg(path);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CPath NLink::getTargetPath() const
{
  if(m_target.get() == CFNULL)
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
  if(node.get() == CFNULL)
    ClientRoot::getLog()->addError("Target is null");
  else
    m_target = node;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::goToTarget()
{
  QModelIndex index = ClientRoot::getTree()->getIndexByPath(m_target->full_path());

  if(index.isValid())
    ClientRoot::getTree()->setCurrentIndex(index);
  else
    ClientRoot::getLog()->addError(QString("%1: path does not exist")
                                   .arg(m_target->full_path().string().c_str()));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::changeTarget()
{
  SelectPathDialog spd;

  CPath path = spd.show(m_target->full_path());

  boost::shared_ptr<XmlDoc> root = XmlOps::create_doc();
  XmlNode * docNode = XmlOps::goto_doc_node(*root.get());

  XmlNode * signal = XmlOps::add_signal_frame(*docNode, "change_link", full_path(),
                           full_path(), true);
  XmlParams p(*signal);

  p.add_param("target_path", path.string());

  ClientRoot::getCore()->sendSignal(*root.get());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::change_link(CF::Common::XmlNode & node)
{
  XmlParams p(node);

  try
  {
    std::string path = p.get_param<std::string>("target_path");

    this->setTargetPath(p.get_param<std::string>("target_path"));

    ClientRoot::getLog()->addMessage(QString("Link '%1' now points to '%2'.")
                                     .arg(full_path().string().c_str()).arg(path.c_str()));

  }
  catch(InvalidPath & ip)
  {
    ClientRoot::getLog()->addError(ip.msg().c_str());
  }
}
