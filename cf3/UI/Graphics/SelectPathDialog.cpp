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

#include "UI/Core/NTree.hpp"
#include "UI/Graphics/TreeView.hpp"

#include "UI/Graphics/SelectPathDialog.hpp"

using namespace cf3::common;
using namespace cf3::UI::Core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

SelectPathDialog::SelectPathDialog(QWidget *parent) :
    QDialog(parent),
    m_okClicked(false),
    m_nodeClicked(false)
{
  this->setWindowTitle("Change target path");

  m_editPath = new QLineEdit(this);
  m_treeView = new TreeView(nullptr, nullptr, false);
  m_buttons = new QDialogButtonBox(this);
  m_completer = new QCompleter(this);
  m_model = new QStringListModel(this);

  m_mainLayout = new QVBoxLayout(this);

  m_completer->setModel(m_model);

  m_completer->setCaseSensitivity(Qt::CaseInsensitive);

  m_editPath->setCompleter(m_completer);

  // create 2 buttons : "Ok" and "Cancel"
  m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok
                                   | QDialogButtonBox::Cancel);

  m_mainLayout->addWidget(m_editPath);
  m_mainLayout->addWidget(m_treeView);
  m_mainLayout->addWidget(m_buttons);

  connect(m_buttons, SIGNAL(accepted()), this, SLOT(btOkClicked()));
  connect(m_buttons, SIGNAL(rejected()), this, SLOT(btCancelClicked()));
  connect(m_treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
  connect(m_editPath, SIGNAL(textChanged(QString)), this, SLOT(pathChanged(QString)));
}

//////////////////////////////////////////////////////////////////////////

URI SelectPathDialog::show(const URI & path)
{
  m_treeView->selectItem(path);
  m_editPath->setText(path.path().c_str());

  m_okClicked = false;

  this->exec();

  if(m_okClicked)
    return m_treeView->selectedPath();
  else
    return URI();
}

//////////////////////////////////////////////////////////////////////////

void SelectPathDialog::btOkClicked()
{
  m_okClicked = true;
  this->setVisible(false);
}

//////////////////////////////////////////////////////////////////////////

void SelectPathDialog::btCancelClicked()
{
  m_okClicked = false;
  this->setVisible(false);
}

//////////////////////////////////////////////////////////////////////////

void SelectPathDialog::itemClicked(const QModelIndex & index)
{
  m_nodeClicked = true;
  m_editPath->setText(m_treeView->selectedPath().path().c_str());
}

//////////////////////////////////////////////////////////////////////////

void SelectPathDialog::pathChanged(const QString & path)
{
  int lastSlash = path.lastIndexOf(URI::separator().c_str());
  QString newPath;
  QStringList list;
  CNode::Ptr node;

  if(m_nodeClicked)
    m_nodeClicked = false;
  else
  {
    CRoot::Ptr root = NTree::globalTree()->treeRoot()->root();
    try
    {
      if(root->retrieve_component<CNode>(path.toStdString()) != nullptr)
        newPath = path;
      else
        newPath = path.left(lastSlash);

      if(newPath == "/")
        newPath = root->uri().path().c_str();

      node = root->retrieve_component<CNode>(newPath.toStdString());

      if(node.get() != nullptr)
      {
        node->listChildPaths(list, false);

        m_model->setStringList(list);

        m_treeView->collapseAll();

        m_treeView->selectItem(newPath.toStdString());
      }
    }
    catch(InvalidURI & ip) {}
  }
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
