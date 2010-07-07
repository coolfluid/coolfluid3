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

  connect(ClientRoot::getTree().get(), SIGNAL(currentIndexChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(currentIndexChanged(const QModelIndex &, const QModelIndex &)));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TreeView::~TreeView()
{
  delete m_modelFilter;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeView::setReadOnly(bool readOnly)
{
  m_readOnly = readOnly;
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
  NTree::Ptr tree = ClientRoot::getTree();

  QModelIndex indexInModel = m_modelFilter->mapToSource(index);

  Qt::MouseButton button = event->button();

//  this->enableDisableOptions(m_treeModel->getParentSimIndex(indexInModel));

  if(event->type() == QEvent::MouseButtonDblClick && button == Qt::LeftButton
    && indexInModel.isValid())
  {
    if(this->isExpanded(index))
      this->collapse(index);
    else
      this->expand(index);
  }
  else if(button == Qt::RightButton)
  {
    if(!tree->getCurrentIndex().isValid())
      tree->setCurrentIndex(indexInModel);

    tree->showNodeMenu(indexInModel, QCursor::pos());
  }
  else if(!tree->areFromSameNode(indexInModel, tree->getCurrentIndex()))
  {
//    if(index != m_treeModel->getCurrentIndex() && this->confirmChangeOptions(index))
    tree->setCurrentIndex(indexInModel);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeView::keyPressEvent(QKeyEvent * event)
{
  NTree::Ptr tree= ClientRoot::getTree();
  QModelIndex currentIndex = m_modelFilter->mapFromSource(tree->getCurrentIndex());

  if(event->key() == Qt::Key_Up)
  {
    if(this->confirmChangeOptions(currentIndex, true))
    {
      QModelIndex above = this->indexAbove(currentIndex);

      if(above.isValid())
        tree->setCurrentIndex(m_modelFilter->mapToSource(above));
    }
  }
  else if(event->key() == Qt::Key_Down)
  {
    if(this->confirmChangeOptions(currentIndex, true))
    {
      QModelIndex below = this->indexBelow(currentIndex);

      if(below.isValid())
        tree->setCurrentIndex(m_modelFilter->mapToSource(below));
    }
  }
  else
    QTreeView::keyPressEvent(event);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TreeView::confirmChangeOptions(const QModelIndex & index, bool okIfSameIndex)
{
  bool confirmed = true;
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

  return confirmed;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeView::currentIndexChanged(const QModelIndex & newIndex, const QModelIndex & oldIndex)
{
  QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Select | QItemSelectionModel::Rows;
  QModelIndex indexInFilter = m_modelFilter->mapFromSource(newIndex);

  this->selectionModel()->clearSelection();
  this->selectionModel()->select(indexInFilter, flags);
  this->selectionModel()->setCurrentIndex(indexInFilter, flags);
}
