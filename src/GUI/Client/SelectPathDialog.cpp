#include <QtGui>

#include "Common/CPath.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/TreeView.hpp"

#include "GUI/Client/SelectPathDialog.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

SelectPathDialog::SelectPathDialog(QWidget *parent) :
    QDialog(parent),
    m_okClicked(false)
{
  this->setWindowTitle("Change target path");

  m_editPath = new QLineEdit(this);
  m_treeView = new TreeView(CFNULL, CFNULL, false);
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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CPath SelectPathDialog::show(const CPath & path)
{
  m_treeView->selectItem(path);
  m_editPath->setText(path.string().c_str());

  m_okClicked = false;

  this->exec();

  if(m_okClicked)
    return m_treeView->getSelectedPath();
  else
    return CPath();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SelectPathDialog::btOkClicked()
{
  m_okClicked = true;
  this->setVisible(false);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SelectPathDialog::btCancelClicked()
{
  m_okClicked = false;
  this->setVisible(false);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SelectPathDialog::itemClicked(const QModelIndex & index)
{
  m_editPath->setText(m_treeView->getSelectedPath().string().c_str());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SelectPathDialog::pathChanged(const QString & path)
{
  int lastSlash = path.lastIndexOf(CPath::separator().c_str());
  QString newPath;
  QStringList list;
  CNode::Ptr node;

  try
  {
    if(ClientRoot::getTree()->getRoot()->root()->access_component<CNode>(path.toStdString()) != CFNULL)
      newPath = path;
    else
      newPath = path.left(lastSlash);

    if(newPath == "/")
      newPath = ClientRoot::getTree()->getRoot()->root()->full_path().string().c_str();

    node = ClientRoot::getTree()->getRoot()->root()->access_component<CNode>(newPath.toStdString());

    if(node.get() != CFNULL)
    {
      node->listChildPaths(list, false);

      m_model->setStringList(list);

      m_treeView->collapseAll();

      m_treeView->selectItem(newPath.toStdString());
    }
  }
  catch(InvalidPath & ip) {}
}
