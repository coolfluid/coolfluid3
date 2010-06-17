#include <QtGui>
#include <QtXml>
#include <QtCore>
#include <stdexcept>

#include "Common/CF.hpp"
#include "Common/Exception.hpp"

#include "GUI/Client/ClientCore.hpp"
#include "GUI/Client/CommitDetails.hpp"
#include "GUI/Client/OptionPanel.hpp"
#include "GUI/Client/CommitDetailsDialog.hpp"
#include "GUI/Client/MenuActionInfo.hpp"
#include "GUI/Client/UnknownTypeException.hpp"
#include "GUI/Client/TObjectProperties.hpp"
#include "GUI/Client/TreeModel.hpp"
#include "GUI/Client/TreeItem.hpp"
#include "GUI/Client/TSshInformation.hpp"
#include "GUI/Client/ConfirmCommitDialog.hpp"
#include "GUI/Client/AddLinkDialog.hpp"
#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/TreeNode.hpp"

#include "GUI/Network/ComponentType.hpp"
#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/TreeView.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;
using namespace CF::GUI::Network;

TreeView::TreeView(OptionPanel * optionsPanel, QMainWindow * parent)
: QTreeView(parent)
{
  MenuActionInfo config;

  if(optionsPanel == CFNULL)
    throw std::invalid_argument("Options panel is a CFNULL pointer");

  // instantiate class attributes
  m_modelFilter = new QSortFilterProxyModel(this);
  m_mnuNewOption = new QMenu("Add an option");

  m_optionsPanel = optionsPanel;

  m_modelFilter->setSourceModel(ClientRoot::getTree().get());
  m_modelFilter->setDynamicSortFilter(true);

  QRegExp reg(QRegExp(".+", Qt::CaseInsensitive, QRegExp::RegExp));
  m_modelFilter->setFilterRegExp(reg);

  this->setModel(m_modelFilter);

  this->setReadOnly(false);

  // when right clicking on the Client,
  // a "Context menu event" must be generated
  this->setContextMenuPolicy(Qt::CustomContextMenu);

  this->header()->setResizeMode(QHeaderView::ResizeToContents);
  this->header()->setStretchLastSection(true);

  connect(this, SIGNAL(activated(const QModelIndex &)),
          this, SLOT(nodeActivated(const QModelIndex &)));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TreeView::~TreeView()
{
  delete m_simulationMenu;
  delete m_objectMenu;
  delete m_mnuAbstractTypes;
  delete m_modelFilter;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeView::setAbstractTypesList(const QStringList & abstractTypes)
{
  QStringList::const_iterator it = abstractTypes.begin();

  // if the menu is not enabled, it can not be modified, even by code.
  m_mnuAbstractTypes->setEnabled(true);

  m_abstractTypesActions.clear();

  // clearing the menu will destroy all its actions (the ones that are not
  // linked to another menu, toolbar, etc...)...so no additional delete is
  // needed
  m_mnuAbstractTypes->clear();

  // create menu m_items for each abstract type
  while(it != abstractTypes.end())
  {
    MenuActionInfo actionInfo;
    QAction * action;

    actionInfo.m_text = *it;
    actionInfo.m_slot = SLOT(addNode());
    actionInfo.m_menu = m_mnuAbstractTypes;

    action = actionInfo.buildAction(this);
    m_abstractTypesActions.append(action);
    it++;
  }

  // if there is at least one abstract type, the sub-menu can be enabled
  m_mnuAbstractTypes->setEnabled(!m_abstractTypesActions.isEmpty());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QDomNode TreeView::newChildNode(const QString & newNode, QDomDocument & doc) const
{
//  QModelIndex index = m_modelFilter->mapToSource(m_treeModel->getCurrentIndex());
//  return m_treeModel->newChildToNode(index, newNode, doc);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeView::setReadOnly(bool readOnly)
{
  m_readOnly = readOnly;
//  m_mnuAbstractTypes->setEnabled(!readOnly);
//  m_mnuNewOption->setEnabled(!readOnly);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TreeView::isReadOnly() const
{
  return m_readOnly;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeView::mousePressEvent(QMouseEvent * event)
{
  QTreeView::mousePressEvent(event);
  QPoint mousePosition(event->x() + this->x(), event->y() + this->y());
  QModelIndex index = this->indexAt(mousePosition);

//  QModelIndex indexInModel = m_modelFilter->mapToSource(index);
  Qt::MouseButton button = event->button();

//  this->enableDisableOptions(m_treeModel->getParentSimIndex(indexInModel));

  if(event->type() == QEvent::MouseButtonDblClick && button == Qt::LeftButton
    && index.isValid())
  {
    if(this->isExpanded(index))
      this->collapse(index);
    else
      this->expand(index);
  }
//  else if(button == Qt::RightButton)
//  {
//    if(!m_treeModel->getCurrentIndex().isValid())
//      m_treeModel->setCurrentIndex(indexInModel);

//    // if this is a simulation node, we display the simulation context menu
//    if(m_treeModel->isSimulationNode(indexInModel) || !indexInModel.isValid())
//      m_simulationMenu->exec(QCursor::pos());
//    else // we display the objects context menu
//      m_objectMenu->exec(QCursor::pos());
//  }
  else //if(!ClientRoot::getTree()->areEqual(indexInModel, ClientRoot::getTree()->getCurrentIndex()))
  {
//    if(index != m_treeModel->getCurrentIndex() && this->confirmChangeOptions(index))
      ClientRoot::getTree()->setCurrentIndex(index);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeView::keyPressEvent(QKeyEvent * event)
{
//  if(event->key() == Qt::Key_Up)
//  {
//    QModelIndex index = m_treeModel->getCurrentIndex();

//    if(this->confirmChangeOptions(index, true))
//    {
//      QModelIndex above = this->indexAbove(m_modelFilter->mapFromSource(index));

//      if(above.isValid())
//        m_treeModel->setCurrentIndex(m_modelFilter->mapToSource(above));
//    }
//  }
//  else if(event->key() == Qt::Key_Down)
//  {
//    QModelIndex index = m_treeModel->getCurrentIndex();

//    if(this->confirmChangeOptions(index, true))
//    {
//      QModelIndex below = this->indexBelow(m_modelFilter->mapFromSource(index));

//      if(below.isValid())
//        m_treeModel->setCurrentIndex(m_modelFilter->mapToSource(below));
//    }
//  }
//  else
//    QTreeView::keyPressEvent(event);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeView::addComponent()
{
  QAction * mnuItem = static_cast<QAction *> (this->sender());
//  QModelIndex index = m_treeModel->getCurrentIndex();
  ComponentType::Type type;
  QString name;

  cf_assert(mnuItem != CFNULL);

  type = ComponentType::Convert::to_enum(mnuItem->text().toStdString());

  if(type == ComponentType::LINK)
  {
    AddLinkDialog * ald = new AddLinkDialog(this);

    QMessageBox::critical(NULL, "Error", "This feature is not available for now");
//    QString name;
//    QModelIndex target;
//    QDomDocument tree;
//
//    m_treeModel->getSimulationTree(m_treeModel->getParentSimIndex(index), tree);
//
//    ald->setTreeModel(tree);
//
//    if(ald->show(m_treeModel->getParentSimIndex(index), target, name))
//      emit addLink(index, name, target);

    delete ald;

  }
  else
  {
    name = QInputDialog::getText(this, tr("New ") + mnuItem->text(),
                               tr("New component name:"), QLineEdit::Normal,
                               "");
//    if(!name.isEmpty())
//      emit addComponent(index, type, name);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TreeView::confirmChangeOptions(const QModelIndex & index, bool okIfSameIndex)
{
//  bool confirmed = true;
//  QMessageBox question(this);

//  if(!okIfSameIndex && m_treeModel->areEqual(m_treeModel->getCurrentIndex(), index))
//    return confirmed;

//  if(m_optionsPanel->isModified())
//  {
//    CommitDetails commitDetails;

//    ConfirmCommitDialog dlg;

//    m_optionsPanel->getModifiedOptions(commitDetails);

//    ConfirmCommitDialog::CommitConfirmation answer = dlg.show(commitDetails);

//    if(answer == ConfirmCommitDialog::COMMIT)
//      m_optionsPanel->commitChanges();

//    confirmed = answer != ConfirmCommitDialog::CANCEL;
//  }

//  return confirmed;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeView::updateTree()
{
  throw NotImplemented(FromHere(), "TreeView::updateTree()");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeView::currentIndexChanged(const QModelIndex & index)
{
//  QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Select | QItemSelectionModel::Rows;
//  QModelIndex indexInFilter = m_modelFilter->mapFromSource(index);

//  this->selectionModel()->clearSelection();
//  this->selectionModel()->select(indexInFilter, flags);
//  this->selectionModel()->setCurrentIndex(indexInFilter, flags);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeView::nodeActivated(const QModelIndex & index)
{

}
