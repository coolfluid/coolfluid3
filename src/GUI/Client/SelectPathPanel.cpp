#include <QCompleter>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QStringListModel>

#include "Common/CPath.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/SelectPathDialog.hpp"

#include "GUI/Client/SelectPathPanel.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

SelectPathPanel::SelectPathPanel(const QString & path, QWidget *parent) :
    QWidget(parent)
{
  m_btBrowse = new QPushButton("Browse", this);
  m_editPath = new QLineEdit(path, this);
  m_completerModel = new QStringListModel(this);
  m_completer = new QCompleter(this);

  m_layout = new QHBoxLayout(this);

  m_editPath->setCompleter(m_completer);
  m_completer->setModel(m_completerModel);

  m_layout->addWidget(m_editPath);
  m_layout->addWidget(m_btBrowse);

  connect(m_btBrowse, SIGNAL(clicked()), this, SLOT(btBrowseClicked()));
  connect(m_editPath, SIGNAL(textChanged(QString)), this, SLOT(updateModel(QString)));

  this->updateModel("");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SelectPathPanel::~SelectPathPanel()
{
  delete m_btBrowse;
  delete m_editPath;
  delete m_completer;
  delete m_layout;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString SelectPathPanel::getValueString() const
{
  return m_editPath->text();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SelectPathPanel::setValue(const QString & path)
{
  m_editPath->setText(path);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SelectPathPanel::btBrowseClicked()
{
  SelectPathDialog spd;

  CPath path = spd.show(m_editPath->text().toStdString());

  if(!path.empty())
    m_editPath->setText(path.string().c_str());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SelectPathPanel::updateModel(const QString & path)
{
  int lastSlash = path.lastIndexOf(CPath::separator().c_str());
  QString newPath;
  QStringList list;
  CNode::Ptr node;

  CRoot::Ptr root = ClientRoot::getTree()->getRoot()->root();

  try
  {
    node = root->access_component<CNode>(path.toStdString());
  }
  catch(InvalidPath & ip) {}

  if(node != CFNULL)
    newPath = path;
  else
    newPath = path.left(lastSlash);

  if(newPath == CPath::separator().c_str())
    newPath = root->full_path().string().c_str();

  try
  {
    if(root->full_path().string() == newPath.toStdString())
      node = ClientRoot::getTree()->getRoot();
    else
      node = root->access_component<CNode>(newPath.toStdString());

    if(node != CFNULL)
    {
      QStringList list;
      node->listChildPaths(list, false, false);

      node = boost::dynamic_pointer_cast<CNode>(node->get_parent());

      if(node != CFNULL)
        node->listChildPaths(list, false, false);

      m_completerModel->setStringList(list);
    }
  }
  catch(InvalidPath & ip) {}

}
