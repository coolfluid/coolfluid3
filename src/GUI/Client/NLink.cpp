#include <QtCore>
#include <QtGui>

#include "Common/CF.hpp"
#include "Common/CPath.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/NLink.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

//QMenu * NLink::m_menu = CFNULL;

NLink::NLink(const QString & name)
  : CNode(name, "CLink", LINK_NODE)
{
  BUILD_COMPONENT;

  //if(m_menu == CFNULL)
  {
    QAction * action;
    //m_menu = new QMenu();

    action = new QAction("Go to target node", m_contextMenu);
    connect(action, SIGNAL(triggered()), this, SLOT(goToTarget()));
    m_contextMenu->addAction(action);

    action = new QAction("Change target path", m_contextMenu);
    connect(action, SIGNAL(triggered()), this, SLOT(changeTarget()));
    m_contextMenu->addAction(action);
  }

//  m_contextMenu = m_menu;
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
  QInputDialog dlg;
  bool selected = false;

  dlg.setLabelText("Enter the new target path:");
  dlg.setTextValue(m_target->full_path().string().c_str());

  while(!selected)
  {
    try
    {
      if(dlg.exec())
      {
        CNode::Ptr node = ClientRoot::getRoot()->root()->access_component<CNode>(dlg.textValue().toStdString());

        if(node->checkType(CNode::LINK_NODE))
          throw InvalidPath(FromHere(), "Can not target another link");
      }

      selected = true;
    }
    catch(InvalidPath ip)
    {
      QMessageBox::critical(CFNULL, "Error", ip.msg().c_str());
    }
  }
}
