#include <QtGui>
#include <QtXml>

#include "Common/CF.hpp"

#include "GUI/Client/TreeItem.hpp"
#include "GUI/Client/TreeModel.hpp"

#include "GUI/Client/AddLinkDialog.hpp"

using namespace CF::GUI::Client;

AddLinkDialog::AddLinkDialog(QWidget * parent)
  : QDialog(parent),
  m_treeModel(NULL),
  m_okCLicked(false)
{
  m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok
                                   | QDialogButtonBox::Cancel, Qt::Horizontal,
                                   this);
  m_treeView = new QTreeView(this);
  m_txtName = new QLineEdit(this);
  m_labName = new QLabel("Name:", this);
  m_labTarget = new QLabel("Target:", this);
  m_mainLayout = new QVBoxLayout(this);

  m_mainLayout->addWidget(m_labName);
  m_mainLayout->addWidget(m_txtName);
  m_mainLayout->addWidget(m_labTarget);
  m_mainLayout->addWidget(m_treeView);
  m_mainLayout->addWidget(m_buttons);

  // connect useful signals to slots
  connect(m_buttons, SIGNAL(accepted()), this, SLOT(btOkClicked()));
  connect(m_buttons, SIGNAL(rejected()), this, SLOT(btCancelClicked()));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

AddLinkDialog::~AddLinkDialog()
{
 delete m_buttons;
 delete m_treeView;
 delete m_labName;
 delete m_labTarget;
 delete m_txtName;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool AddLinkDialog::show(const QModelIndex & root, QModelIndex & index,
                         QString & name)
{
  cf_assert(m_treeModel != CFNULL);
  cf_assert(root.isValid());

  qDebug() << static_cast<TreeItem*>(root.internalPointer())->getDomNode().nodeName();

  //m_treeView->setRootIndex(root);
 // m_treeView->setVisible(true);

  this->exec();

  if(m_okCLicked)
  {
    index = m_treeView->currentIndex();
    name = m_txtName->text();
  }

  return m_okCLicked;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void AddLinkDialog::setTreeModel(const QDomDocument & tree/*TreeModel * treeModel*/)
{
  cf_assert(!tree.firstChild().isNull()); // the tree can not be empty

  //if(m_treeModel != treeModel)
  {
    m_treeView->setModel(NULL);
    delete m_treeModel;

    m_treeModel = new TreeModel(tree);
    m_treeView->setModel(m_treeModel);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void AddLinkDialog::btOkClicked()
{
  if(!m_treeView->currentIndex().isValid() && m_txtName->text().isEmpty())
  {
    QMessageBox::critical(this, "Error", "Please enter a non-empty name and "
                          "selected a component.");
  }
  else
  {
   m_okCLicked = true;
   this->hide();
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void AddLinkDialog::btCancelClicked()
{
  m_okCLicked = false;
  this->hide();
}
