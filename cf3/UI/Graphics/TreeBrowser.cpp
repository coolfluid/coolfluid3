// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QGridLayout>
#include <QMenu>
#include <QPersistentModelIndex>
#include <QPushButton>
#include <QLineEdit>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include "common/URI.hpp"

#include "UI/Core/NTree.hpp"

#include "UI/Graphics/TreeView.hpp"

#include "UI/Graphics/TreeBrowser.hpp"

using namespace cf3::common;
using namespace cf3::UI::Core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

TreeBrowser::TreeBrowser(TreeView * view, QWidget *parent) :
    QWidget(parent),
    m_tree_view(view),
    m_current_index(0)
{
  cf3_assert(view != nullptr);

  m_bt_previous = new QToolButton(this);
  m_bt_next = new QToolButton(this);
  m_menu_next = new QMenu(m_bt_next);
  m_menu_previous = new QMenu(m_bt_previous);
  m_filter = new QLineEdit(this);

  m_buttons_layout = new QGridLayout();
  m_main_layout = new QVBoxLayout(this);

  m_main_layout->setMargin(0);

  m_bt_previous->setArrowType(Qt::LeftArrow);
  m_bt_next->setArrowType(Qt::RightArrow);

  m_bt_next->setMenu(m_menu_next);
  m_bt_previous->setMenu(m_menu_previous);

  m_bt_next->setPopupMode(QToolButton::MenuButtonPopup);
  m_bt_previous->setPopupMode(QToolButton::MenuButtonPopup);

  m_buttons_layout->addWidget(m_bt_previous, 0, 0);
  m_buttons_layout->addWidget(m_bt_next, 0, 1);
  m_buttons_layout->addWidget(m_filter, 0, 2);

  m_buttons_layout->setColumnStretch(2, 10);

  m_main_layout->addLayout(m_buttons_layout);
  m_main_layout->addWidget(m_tree_view);

  connect(m_bt_previous, SIGNAL(clicked()), this, SLOT(previous_clicked()));
  connect(m_bt_next, SIGNAL(clicked()), this, SLOT(next_clicked()));
  connect(m_filter, SIGNAL(textChanged(QString)), this, SLOT(filter_updated(QString)));

  connect(m_tree_view, SIGNAL(item_double_clicked(QModelIndex)),
          this, SLOT(double_clicked(QModelIndex)));

  m_history << m_tree_view->rootIndex();
  this->update_buttons();
}

//////////////////////////////////////////////////////////////////////////

TreeBrowser::~TreeBrowser()
{
  delete m_menu_next;
  delete m_menu_previous;
  delete m_buttons_layout;
  delete m_main_layout;
  delete m_bt_next;
  delete m_bt_previous;
  delete m_filter;
}

//////////////////////////////////////////////////////////////////////////

void TreeBrowser::focus_filter()
{
  m_filter->setFocus();
}

//////////////////////////////////////////////////////////////////////////

void TreeBrowser::previous_clicked()
{
  cf3_assert(m_current_index > 0);

  m_current_index--;

  const QModelIndex & index = m_history.at(m_current_index);

//  if(!index.parent().isValid())
//    m_treeView->setRootIndex(index.parent());
//  else
    m_tree_view->setRootIndex(index.parent());

  this->update_buttons();
}

//////////////////////////////////////////////////////////////////////////

void TreeBrowser::next_clicked()
{
  cf3_assert(m_current_index < m_history.size() - 1);

  m_current_index++;
  m_tree_view->setRootIndex(m_history.at(m_current_index));
  this->update_buttons();
}

//////////////////////////////////////////////////////////////////////////

void TreeBrowser::double_clicked(const QModelIndex & index)
{
  while(m_current_index < m_history.size() - 1)
    m_history.removeLast();

  m_history << QPersistentModelIndex(index);
  //m_actions << QAction(m_treeView->getPath(index).string().c_string());
  m_current_index++;
  m_tree_view->setRootIndex(index);
  this->update_buttons();
}

//////////////////////////////////////////////////////////////////////////

void TreeBrowser::action_triggered()
{
  QAction * action = static_cast<QAction*>(sender());

  if(action != nullptr && m_actions.contains(action))
  {
    m_current_index = m_actions[action];
    m_tree_view->setRootIndex(m_history.at(m_current_index));
    this->update_buttons();
  }
}

//////////////////////////////////////////////////////////////////////////

void TreeBrowser::filter_updated(const QString & text)
{
  m_tree_view->set_filter(text);
}

//////////////////////////////////////////////////////////////////////////

void TreeBrowser::update_buttons()
{
  m_actions.clear();
  m_menu_next->clear();
  m_menu_previous->clear();

  if(!m_history.isEmpty())
  {
    for(int i = 0 ; i < m_history.size() ; i++)
    {
      QPersistentModelIndex index = m_history.at(i);
      QString path = m_tree_view->path_from_index(index).path().c_str();
      QIcon icon = m_tree_view->icon_from_index(index);

      if(path.isEmpty())
      {
        path = NTree::global()->tree_root()->root()->uri().path().c_str();
        //icon = NTree::global()->getRoot()->getIcon();
      }

      QAction * action = new QAction(icon, path, this);

      connect(action, SIGNAL(triggered()), this, SLOT(action_triggered()));
      m_actions[action] = i;

        if(i < m_current_index)
          m_menu_previous->addAction(action);
        else if(i > m_current_index)
          m_menu_next->addAction(action);
    }
  }

  m_bt_next->setEnabled(!m_menu_next->isEmpty());
  m_bt_previous->setEnabled(!m_menu_previous->isEmpty());
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
