// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QHeaderView>
#include <QMainWindow>
#include <QModelIndex>
#include <QMouseEvent>
#include <QRegExp>
#include <QSortFilterProxyModel>
#include <QDebug>

#include "Common/CF.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/CommitDetails.hpp"
#include "GUI/Client/UI/CommitDetailsDialog.hpp"
#include "GUI/Client/UI/ConfirmCommitDialog.hpp"
#include "GUI/Client/UI/OptionPanel.hpp"
#include "GUI/Client/Core/UnknownTypeException.hpp"

#include "GUI/Network/ComponentType.hpp"
#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/UI/TreeView.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;
using namespace CF::GUI::Network;

TreeView::TreeView(OptionPanel * optionsPanel, QMainWindow * parent,
                   bool contextMenuAllowed)
: QTreeView(parent),
  m_contextMenuAllowed(contextMenuAllowed)
{
  if(m_contextMenuAllowed && optionsPanel == CFNULL)
    throw BadValue(FromHere(), "Options panel is a CFNULL pointer");

  // instantiate class attributes
  m_modelFilter = new QSortFilterProxyModel(this);

  m_optionsPanel = optionsPanel;

  m_modelFilter->setSourceModel(ClientRoot::tree().get());
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

  if(m_contextMenuAllowed)
  {
    connect(ClientRoot::tree().get(),
            SIGNAL(currentIndexChanged(QModelIndex, QModelIndex)),
            this,
            SLOT(currentIndexChanged(QModelIndex, QModelIndex)));
  }
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

CPath TreeView::getSelectedPath() const
{
  QModelIndex currentPath = this->selectionModel()->currentIndex();
  CPath path;

  if(currentPath.isValid())
  {
    QModelIndex indexInModel = m_modelFilter->mapToSource(currentPath);

    path = ClientRoot::tree()->getIndexPath(indexInModel);
  }

  return path;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CPath TreeView::getPath(const QModelIndex & index)
{
  return ClientRoot::tree()->getIndexPath(m_modelFilter->mapToSource(index));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon TreeView::getIcon(const QModelIndex & index)
{
  QModelIndex indexInModel = m_modelFilter->mapToSource(index);
  return ClientRoot::tree()->data(indexInModel, Qt::DecorationRole).value<QIcon>();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeView::selectItem(const CPath & path)
{
  QModelIndex index = ClientRoot::tree()->getIndexByPath(path);

  if(index.isValid())
  {
    QItemSelectionModel::SelectionFlags flags;
    QModelIndex indexInFilter = m_modelFilter->mapFromSource(index);

    flags = QItemSelectionModel::Select | QItemSelectionModel::Rows;

    this->selectionModel()->clearSelection();
    this->selectionModel()->select(indexInFilter, flags);
    this->selectionModel()->setCurrentIndex(indexInFilter, flags);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeView::mousePressEvent(QMouseEvent * event)
{
  QTreeView::mousePressEvent(event);
  QPoint mousePosition = event->pos() + this->geometry().topLeft();//(event->x() + this->geometry().x(), event->y() + this->geometry().y());
  QModelIndex index = this->indexAt(mousePosition);
  NTree::Ptr tree = ClientRoot::tree();

  QTreeView::mousePressEvent(event);

  QModelIndex indexInModel = m_modelFilter->mapToSource(this->currentIndex());

  Qt::MouseButton button = event->button();

  try
  {
    if(m_contextMenuAllowed && indexInModel.isValid())
    {
      if(button == Qt::RightButton)
      {
//        if(!tree->getCurrentIndex().isValid())
          tree->setCurrentIndex(indexInModel);

        ClientRoot::log()->addMessage(tree->getNodePath(tree->getCurrentIndex()) + " " +
                                      tree->getNodePath(indexInModel));

        tree->showNodeMenu(indexInModel, QCursor::pos());
      }
      else if(!tree->areFromSameNode(indexInModel, tree->getCurrentIndex()))
      {
        if(this->confirmChangeOptions(index))
          tree->setCurrentIndex(indexInModel);
        else
          this->currentIndexChanged(tree->getCurrentIndex(), tree->getCurrentIndex());
      }
    }
  }
  catch(Exception & e)
  {
    ClientRoot::log()->addException(e.what());
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeView::keyPressEvent(QKeyEvent * event)
{
  NTree::Ptr tree= ClientRoot::tree();
  QModelIndex currentIndex = m_modelFilter->mapFromSource(tree->getCurrentIndex());

  if(m_contextMenuAllowed)
  {
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
  else
  {
    QTreeView::keyPressEvent(event);
    emit clicked(this->selectionModel()->currentIndex());
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TreeView::confirmChangeOptions(const QModelIndex & index, bool okIfSameIndex)
{
  bool confirmed = true;
  NTree::Ptr tree = ClientRoot::tree();

  if(!okIfSameIndex &&  tree->areFromSameNode(tree->getCurrentIndex(), index))
    return confirmed;

  if(m_optionsPanel->isModified())
  {
    CommitDetails commitDetails;

    ConfirmCommitDialog dlg;

    m_optionsPanel->getModifiedOptions(commitDetails);

    ConfirmCommitDialog::CommitConfirmation answer = dlg.show(commitDetails);

    if(answer == ConfirmCommitDialog::COMMIT)
      m_optionsPanel->btApplyClicked();

    confirmed = answer != ConfirmCommitDialog::CANCEL;
  }

  return confirmed;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeView::currentIndexChanged(const QModelIndex & newIndex,
                                   const QModelIndex & oldIndex)
{
  if(m_contextMenuAllowed)
  {
    QItemSelectionModel::SelectionFlags flags;
    QModelIndex indexInFilter = m_modelFilter->mapFromSource(newIndex);

    flags = QItemSelectionModel::Select | QItemSelectionModel::Rows;

    this->selectionModel()->clearSelection();
    this->selectionModel()->select(indexInFilter, flags);
    this->selectionModel()->setCurrentIndex(indexInFilter, flags);
  }
}
