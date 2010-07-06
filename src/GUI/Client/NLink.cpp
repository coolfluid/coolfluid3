#include <QtCore>
#include <QtGui>

#include "Common/CF.hpp"
#include "Common/CPath.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/NLink.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

NLink::NLink(const QString & name, const CPath & targetPath)
  : CNode(name, "CLink", LINK_NODE),
    m_targetPath(targetPath)
{
  BUILD_COMPONENT;

  QAction * action;

  action = new QAction("Go to target node", m_contextMenu);
  connect(action, SIGNAL(triggered()), this, SLOT(goToTarget()));
  m_contextMenu->addAction(action);
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
  return QString("Target: %1").arg(m_targetPath.string().c_str());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::getOptions(QList<NodeOption> & params) const
{
  CNode::Ptr target = ClientRoot::getTree()->getNodeByPath(m_targetPath);

  if(target.get() != CFNULL)
    target->getOptions(params);
  else
    ClientRoot::getLog()->addError(QString("%1: path does not exist").arg(m_targetPath.string().c_str()));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CF::Common::CPath NLink::getTargetPath() const
{
  return m_targetPath;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::goToTarget()
{
  QModelIndex index = ClientRoot::getTree()->getIndexByPath(m_targetPath);

  if(index.isValid())
    ClientRoot::getTree()->setCurrentIndex(index);
  else
    ClientRoot::getLog()->addError(QString("%1: path does not exist").arg(m_targetPath.string().c_str()));
}
