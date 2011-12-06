// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QCompleter>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QModelIndex>
#include <QStringListModel>
#include <QVBoxLayout>

#include "common/URI.hpp"

#include "ui/core/NTree.hpp"
#include "ui/graphics/TreeView.hpp"

#include "ui/graphics/SelectPathDialog.hpp"

using namespace cf3::common;
using namespace cf3::ui::core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

SelectPathDialog::SelectPathDialog(QWidget *parent) :
    QDialog(parent),
    m_ok_clicked(false),
    m_node_clicked(false)
{
  this->setWindowTitle("Change target path");

  m_edit_path = new QLineEdit(this);
  m_tree_view = new TreeView(nullptr, nullptr, false);
  m_buttons = new QDialogButtonBox(this);
  m_completer = new QCompleter(this);
  m_model = new QStringListModel(this);

  m_main_layout = new QVBoxLayout(this);

  m_completer->setModel(m_model);

  m_completer->setCaseSensitivity(Qt::CaseInsensitive);

  m_edit_path->setCompleter(m_completer);

  // create 2 buttons : "Ok" and "Cancel"
  m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok
                                   | QDialogButtonBox::Cancel);

  m_main_layout->addWidget(m_edit_path);
  m_main_layout->addWidget(m_tree_view);
  m_main_layout->addWidget(m_buttons);

  connect(m_buttons, SIGNAL(accepted()), this, SLOT(bt_ok_clicked()));
  connect(m_buttons, SIGNAL(rejected()), this, SLOT(bt_cancel_clicked()));
  connect(m_tree_view, SIGNAL(clicked(QModelIndex)), this, SLOT(item_clicked(QModelIndex)));
  connect(m_edit_path, SIGNAL(textChanged(QString)), this, SLOT(path_changed(QString)));
}

//////////////////////////////////////////////////////////////////////////

URI SelectPathDialog::show(const URI & path)
{
  m_tree_view->select_item(path);
  m_edit_path->setText(path.path().c_str());

  m_ok_clicked = false;

  this->exec();

  if(m_ok_clicked)
    return m_tree_view->selected_path();
  else
    return URI();
}

//////////////////////////////////////////////////////////////////////////

void SelectPathDialog::bt_ok_clicked()
{
  m_ok_clicked = true;
  this->setVisible(false);
}

//////////////////////////////////////////////////////////////////////////

void SelectPathDialog::bt_cancel_clicked()
{
  m_ok_clicked = false;
  this->setVisible(false);
}

//////////////////////////////////////////////////////////////////////////

void SelectPathDialog::item_clicked(const QModelIndex & index)
{
  m_node_clicked = true;
  m_edit_path->setText(m_tree_view->selected_path().path().c_str());
}

//////////////////////////////////////////////////////////////////////////

void SelectPathDialog::path_changed(const QString & path)
{
  int lastSlash = path.lastIndexOf(URI::separator().c_str());
  QString newPath;
  QStringList list;
  Handle< CNode > node;

  if(m_node_clicked)
    m_node_clicked = false;
  else
  {
    Handle< Component > root = NTree::global()->tree_root();
    try
    {
      if(Handle<CNode>(root->access_component(path.toStdString())) != nullptr)
        newPath = path;
      else
        newPath = path.left(lastSlash);

      if(newPath == "/")
        newPath = root->uri().path().c_str();

      node = Handle<CNode>(root->access_component(newPath.toStdString()));

      if(node.get() != nullptr)
      {
        node->list_child_paths(list, false);

        m_model->setStringList(list);

        m_tree_view->collapseAll();

        m_tree_view->select_item(newPath.toStdString());
      }
    }
    catch(InvalidURI & ip) {}
  }
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
