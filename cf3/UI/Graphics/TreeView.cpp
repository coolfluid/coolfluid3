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

#include "common/CF.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/CommitDetails.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/Core/NTree.hpp"

#include "UI/Graphics/FilteringModel.hpp"
#include "UI/Graphics/ConfirmCommitDialog.hpp"
#include "UI/Graphics/CentralPanel.hpp"
#include "UI/Graphics/SignalManager.hpp"

#include "UI/Graphics/TreeView.hpp"

using namespace cf3::common;
using namespace cf3::UI::Core;
using namespace cf3::UI::UICommon;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

TreeView::TreeView(CentralPanel * optionsPanel, QMainWindow * parent,
                   bool contextMenuAllowed)
: QTreeView(parent),
  m_context_menu_allowed(contextMenuAllowed)
{
  if(m_context_menu_allowed && optionsPanel == nullptr)
    throw BadValue(FromHere(), "Options panel is a nullptr pointer");

  // instantiate class attributes
  m_model_filter = new FilteringModel(this);
  m_central_panel = optionsPanel;
  m_signal_manager = new SignalManager(parent);

  m_model_filter->setSourceModel(NTree::global().get());
  m_model_filter->setDynamicSortFilter(true);

  this->setModel(m_model_filter);

  this->set_read_only(false);

  // when right clicking on the Client,
  // a "Context menu event" must be generated
  this->setContextMenuPolicy(Qt::CustomContextMenu);

  this->header()->setResizeMode(QHeaderView::ResizeToContents);
  this->header()->setStretchLastSection(true);

  if(m_context_menu_allowed)
  {
    connect(NTree::global().get(),
            SIGNAL(current_index_changed(QModelIndex, QModelIndex)),
            this,
            SLOT(current_index_changed(QModelIndex, QModelIndex)));
  }
}

//////////////////////////////////////////////////////////////////////////

TreeView::~TreeView()
{
  delete m_model_filter;
}

//////////////////////////////////////////////////////////////////////////

void TreeView::set_read_only(bool readOnly)
{
  m_read_only = readOnly;
}

//////////////////////////////////////////////////////////////////////////

bool TreeView::is_read_only() const
{
  return m_read_only;
}

//////////////////////////////////////////////////////////////////////////

URI TreeView::selected_path() const
{
  QModelIndex currentPath = this->selectionModel()->currentIndex();
  URI path;

  if(currentPath.isValid())
  {
    QModelIndex indexInModel = m_model_filter->mapToSource(currentPath);

    path = NTree::global()->pathFromIndex(indexInModel);
  }

  return path;
}

//////////////////////////////////////////////////////////////////////////

URI TreeView::path_from_index(const QModelIndex & index)
{
  return NTree::global()->pathFromIndex(m_model_filter->mapToSource(index));
}

//////////////////////////////////////////////////////////////////////////

QIcon TreeView::icon_from_index(const QModelIndex & index)
{
  QModelIndex indexInModel = m_model_filter->mapToSource(index);
  return NTree::global()->data(indexInModel, Qt::DecorationRole).value<QIcon>();
}

//////////////////////////////////////////////////////////////////////////

void TreeView::select_item(const URI & path)
{
  QModelIndex index = NTree::global()->index_from_path(path);

  if(index.isValid())
  {
    QItemSelectionModel::SelectionFlags flags;
    QModelIndex indexInFilter = m_model_filter->mapFromSource(index);

    flags = QItemSelectionModel::Select | QItemSelectionModel::Rows;

    this->selectionModel()->clearSelection();
    this->selectionModel()->select(indexInFilter, flags);
    this->selectionModel()->setCurrentIndex(indexInFilter, flags);
  }
}

//////////////////////////////////////////////////////////////////////////

void TreeView::set_filter(const QString & pattern)
{
  QRegExp regex(pattern, Qt::CaseInsensitive, QRegExp::Wildcard);
  m_model_filter->setFilterRegExp(regex);
  this->expandAll();
}

//////////////////////////////////////////////////////////////////////////

bool TreeView::try_commit()
{
  bool committed = true;
  QModelIndex currentIndex = NTree::global()->current_index();

  if(currentIndex.isValid())
  {
    committed = confirm_change_options(currentIndex, true);
  }

  return committed;
}

//////////////////////////////////////////////////////////////////////////

void TreeView::mousePressEvent(QMouseEvent * event)
{
  QTreeView::mousePressEvent(event);
  QPoint mousePosition = event->pos() + this->geometry_fields().topLeft();

  QModelIndex index = this->indexAt(mousePosition);
  NTree::Ptr tree = NTree::global();

  QModelIndex indexInModel = m_model_filter->mapToSource(this->currentIndex());

  Qt::MouseButton button = event->button();

  try
  {
    if(m_context_menu_allowed && indexInModel.isValid())
    {
      if(button == Qt::RightButton && this->confirm_change_options(index))
      {
        QList<ActionInfo> actions;
        URI path;

        tree->set_current_index(indexInModel);
        tree->list_node_actions(indexInModel, actions);
        path =  tree->current_path();

        m_signal_manager->show_menu(QCursor::pos(), tree->node_by_path(path), actions);
      }
      else if(!tree->are_from_same_node(indexInModel, tree->current_index()))
      {
        if(this->confirm_change_options(index))
          tree->set_current_index(indexInModel);
        else
          this->current_index_changed(tree->current_index(), tree->current_index());
      }
    }
  }
  catch(Exception & e)
  {
    NLog::global()->add_exception(e.what());
  }
}

//////////////////////////////////////////////////////////////////////////

void TreeView::mouseDoubleClickEvent(QMouseEvent * event)
{
  if(event->button() == Qt::LeftButton)
    emit item_double_clicked(this->currentIndex());
}

//////////////////////////////////////////////////////////////////////////

void TreeView::keyPressEvent(QKeyEvent * event)
{
  NTree::Ptr tree= NTree::global();
  QModelIndex currentIndex = m_model_filter->mapFromSource(tree->current_index());

  if(m_context_menu_allowed)
  {
    if(event->key() == Qt::Key_Up)
    {
      if(this->confirm_change_options(currentIndex, true))
      {
        QModelIndex above = this->indexAbove(currentIndex);

        if(above.isValid())
          tree->set_current_index(m_model_filter->mapToSource(above));
      }
    }
    else if(event->key() == Qt::Key_Down)
    {
      if(this->confirm_change_options(currentIndex, true))
      {
        QModelIndex below = this->indexBelow(currentIndex);

        if(below.isValid())
          tree->set_current_index(m_model_filter->mapToSource(below));
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

bool TreeView::confirm_change_options(const QModelIndex & index, bool okIfSameIndex)
{
  bool confirmed = true;
  NTree::Ptr tree = NTree::global();

  if(!okIfSameIndex &&  tree->are_from_same_node(tree->current_index(), index))
    return confirmed;

  if(m_central_panel->is_modified())
  {
    CommitDetails commitDetails;

    ConfirmCommitDialog dlg;

    m_central_panel->list_modified_options(commitDetails);

    ConfirmCommitDialog::CommitConfirmation answer = dlg.show(commitDetails);

    if(answer == ConfirmCommitDialog::COMMIT)
      m_central_panel->bt_apply_clicked();

    confirmed = answer != ConfirmCommitDialog::CANCEL;
  }

  return confirmed;
}

//////////////////////////////////////////////////////////////////////////

void TreeView::current_index_changed(const QModelIndex & newIndex,
                                   const QModelIndex & oldIndex)
{
  if(m_context_menu_allowed)
  {
    QItemSelectionModel::SelectionFlags flags;
    QModelIndex indexInFilter = m_model_filter->mapFromSource(newIndex);

    flags = QItemSelectionModel::Select | QItemSelectionModel::Rows;

    this->selectionModel()->clearSelection();
    this->selectionModel()->select(indexInFilter, flags);
    this->selectionModel()->setCurrentIndex(indexInFilter, flags);
  }
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
