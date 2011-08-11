// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

#include "Common/CF.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/CommitDetails.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/Core/NTree.hpp"

#include "UI/Graphics/FilteringModel.hpp"
#include "UI/Graphics/ConfirmCommitDialog.hpp"
#include "UI/Graphics/CentralPanel.hpp"
#include "UI/Graphics/SignalManager.hpp"

#include "UI/Graphics/TreeView.hpp"

using namespace CF::Common;
using namespace CF::UI::Core;
using namespace CF::UI::UICommon;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

TreeView::TreeView(CentralPanel * optionsPanel, QMainWindow * parent,
                   bool contextMenuAllowed)
: QTreeView(parent),
  m_contextMenuAllowed(contextMenuAllowed)
{
  if(m_contextMenuAllowed && optionsPanel == nullptr)
    throw BadValue(FromHere(), "Options panel is a nullptr pointer");

  // instantiate class attributes
  m_modelFilter = new FilteringModel(this);
  m_centralPanel = optionsPanel;
  m_signalManager = new SignalManager(parent);

  m_modelFilter->setSourceModel(NTree::globalTree().get());
  m_modelFilter->setDynamicSortFilter(true);

  this->setModel(m_modelFilter);

  this->setReadOnly(false);

  // when right clicking on the Client,
  // a "Context menu event" must be generated
  this->setContextMenuPolicy(Qt::CustomContextMenu);

  this->header()->setResizeMode(QHeaderView::ResizeToContents);
  this->header()->setStretchLastSection(true);

  if(m_contextMenuAllowed)
  {
    connect(NTree::globalTree().get(),
            SIGNAL(currentIndexChanged(QModelIndex, QModelIndex)),
            this,
            SLOT(currentIndexChanged(QModelIndex, QModelIndex)));
  }
}

//////////////////////////////////////////////////////////////////////////

TreeView::~TreeView()
{
  delete m_modelFilter;
}

//////////////////////////////////////////////////////////////////////////

void TreeView::setReadOnly(bool readOnly)
{
  m_readOnly = readOnly;
}

//////////////////////////////////////////////////////////////////////////

bool TreeView::isReadOnly() const
{
  return m_readOnly;
}

//////////////////////////////////////////////////////////////////////////

URI TreeView::selectedPath() const
{
  QModelIndex currentPath = this->selectionModel()->currentIndex();
  URI path;

  if(currentPath.isValid())
  {
    QModelIndex indexInModel = m_modelFilter->mapToSource(currentPath);

    path = NTree::globalTree()->pathFromIndex(indexInModel);
  }

  return path;
}

//////////////////////////////////////////////////////////////////////////

URI TreeView::pathFromIndex(const QModelIndex & index)
{
  return NTree::globalTree()->pathFromIndex(m_modelFilter->mapToSource(index));
}

//////////////////////////////////////////////////////////////////////////

QIcon TreeView::iconFromIndex(const QModelIndex & index)
{
  QModelIndex indexInModel = m_modelFilter->mapToSource(index);
  return NTree::globalTree()->data(indexInModel, Qt::DecorationRole).value<QIcon>();
}

//////////////////////////////////////////////////////////////////////////

void TreeView::selectItem(const URI & path)
{
  QModelIndex index = NTree::globalTree()->indexFromPath(path);

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

//////////////////////////////////////////////////////////////////////////

void TreeView::setFilter(const QString & pattern)
{
  QRegExp regex(pattern, Qt::CaseInsensitive, QRegExp::Wildcard);
  m_modelFilter->setFilterRegExp(regex);
  this->expandAll();
}

//////////////////////////////////////////////////////////////////////////

bool TreeView::tryToCommit()
{
  bool committed = true;
  QModelIndex currentIndex = NTree::globalTree()->currentIndex();

  if(currentIndex.isValid())
  {
    committed = confirmChangeOptions(currentIndex, true);
  }

  return committed;
}

//////////////////////////////////////////////////////////////////////////

void TreeView::mousePressEvent(QMouseEvent * event)
{
  QTreeView::mousePressEvent(event);
  QPoint mousePosition = event->pos() + this->geometry().topLeft();

  QModelIndex index = this->indexAt(mousePosition);
  NTree::Ptr tree = NTree::globalTree();

  QModelIndex indexInModel = m_modelFilter->mapToSource(this->currentIndex());

  Qt::MouseButton button = event->button();

  try
  {
    if(m_contextMenuAllowed && indexInModel.isValid())
    {
      if(button == Qt::RightButton && this->confirmChangeOptions(index))
      {
        QList<ActionInfo> actions;
        URI path;

        tree->setCurrentIndex(indexInModel);
        tree->listNodeActions(indexInModel, actions);
        path =  tree->currentPath();

        m_signalManager->showMenu(QCursor::pos(), tree->nodeByPath(path), actions);
      }
      else if(!tree->areFromSameNode(indexInModel, tree->currentIndex()))
      {
        if(this->confirmChangeOptions(index))
          tree->setCurrentIndex(indexInModel);
        else
          this->currentIndexChanged(tree->currentIndex(), tree->currentIndex());
      }
    }
  }
  catch(Exception & e)
  {
    NLog::globalLog()->addException(e.what());
  }
}

//////////////////////////////////////////////////////////////////////////

void TreeView::mouseDoubleClickEvent(QMouseEvent * event)
{
  if(event->button() == Qt::LeftButton)
    emit itemDoubleClicked(this->currentIndex());
}

//////////////////////////////////////////////////////////////////////////

void TreeView::keyPressEvent(QKeyEvent * event)
{
  NTree::Ptr tree= NTree::globalTree();
  QModelIndex currentIndex = m_modelFilter->mapFromSource(tree->currentIndex());

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

//////////////////////////////////////////////////////////////////////////

bool TreeView::confirmChangeOptions(const QModelIndex & index, bool okIfSameIndex)
{
  bool confirmed = true;
  NTree::Ptr tree = NTree::globalTree();

  if(!okIfSameIndex &&  tree->areFromSameNode(tree->currentIndex(), index))
    return confirmed;

  if(m_centralPanel->isModified())
  {
    CommitDetails commitDetails;

    ConfirmCommitDialog dlg;

    m_centralPanel->modifiedOptions(commitDetails);

    ConfirmCommitDialog::CommitConfirmation answer = dlg.show(commitDetails);

    if(answer == ConfirmCommitDialog::COMMIT)
      m_centralPanel->btApplyClicked();

    confirmed = answer != ConfirmCommitDialog::CANCEL;
  }

  return confirmed;
}

//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF
