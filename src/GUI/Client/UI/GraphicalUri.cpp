// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QComboBox>
#include <QCompleter>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QStringListModel>

#include "Common/CPath.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/UI/SelectPathDialog.hpp"
#include "GUI/Client/UI/NRemoteOpen.hpp"

#include "GUI/Client/UI/GraphicalUri.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientUI;

GraphicalUri::GraphicalUri(CF::Common::OptionURI::ConstPtr opt, QWidget *parent) :
    GraphicalValue(parent)
{
  m_btBrowse = new QPushButton("Browse", this);
  m_editPath = new QLineEdit(this);
  m_completerModel = new QStringListModel(this);
  m_completer = new QCompleter(this);
  m_comboType = new QComboBox(this);

  m_completer->setModel(m_completerModel);

  m_comboType->addItems(QStringList() << "cpath" << "file" << "http");

  changeType(m_comboType->currentText());

  m_layout->addWidget(m_comboType);
  m_layout->addWidget(m_editPath);
  m_layout->addWidget(m_btBrowse);

  this->setValue(opt->value_str().c_str());
  this->setProtocols(opt->supported_protocols());

  connect(m_btBrowse, SIGNAL(clicked()), this, SLOT(btBrowseClicked()));
//  connect(m_editPath, SIGNAL(textChanged(QString)), this, SLOT(updateModel(QString)));
  connect(m_comboType, SIGNAL(activated(QString)), this, SLOT(changeType(QString)));

  this->updateModel("");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GraphicalUri::~GraphicalUri()
{
  delete m_btBrowse;
  delete m_editPath;
  delete m_completer;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalUri::getValue() const
{
  return m_editPath->text();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalUri::setProtocols(const std::vector<std::string> & list)
{
  m_comboType->clear();

  if(list.empty())
  {
    m_comboType->addItem("cpath");
    m_comboType->addItem("file");
    m_comboType->addItem("http");
  }
  else
  {
    std::vector<std::string>::const_iterator it;
    for(it = list.begin() ; it != list.end() ; it++)
      m_comboType->addItem(it->c_str());
  }

  changeType(m_comboType->currentText());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalUri::setValue(const QVariant & value)
{
  if(value.type() == QVariant::String)
  {
    m_originalValue = value;
    m_editPath->setText(value.toString());
    return true;
  }

  return false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalUri::btBrowseClicked()
{
  if(m_comboType->currentText() == "cpath")
  {
    SelectPathDialog spd;
    QString modified_path = m_editPath->text();

    CPath path = spd.show(modified_path.toStdString());

    if(!path.empty())
      m_editPath->setText(path.string().c_str());
  }
  else if(m_comboType->currentText() == "file")
  {
    NRemoteOpen::Ptr nro = NRemoteOpen::create();
    bool canceled;

    QString filename = nro->show("", &canceled);

    if(!canceled && !filename.isEmpty())
      m_editPath->setText(filename);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalUri::updateModel(const QString & path)
{
  if(m_editPath->completer() == nullptr)
    return;

  int lastSlash = path.lastIndexOf(CPath::separator().c_str());
  QString newPath;
  QStringList list;
  CNode::Ptr node;

  CRoot::Ptr root = ClientRoot::tree()->getRoot()->root();

  try
  {
    node = root->access_component<CNode>(path.toStdString());
  }
  catch(InvalidPath & ip) {}

  if(node != nullptr)
    newPath = path;
  else
    newPath = path.left(lastSlash);

  if(newPath == CPath::separator().c_str())
    newPath = root->full_path().string().c_str();

  try
  {
    if(root->full_path().string() == newPath.toStdString())
      node = ClientRoot::tree()->getRoot();
    else
      node = root->access_component<CNode>(newPath.toStdString());

    if(node != nullptr)
    {
      QStringList list;
      node->listChildPaths(list, false, false);

      node = boost::dynamic_pointer_cast<CNode>(node->get_parent());

      if(node != nullptr)
        node->listChildPaths(list, false, false);

      m_completerModel->setStringList(list);
    }
  }
  catch(InvalidPath & ip) {}

  emit valueChanged();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalUri::changeType(const QString & type)
{
  m_btBrowse->setVisible(type == "cpath" || type == "file");
  m_editPath->setVisible(!type.isEmpty());
  m_editPath->setCompleter(type == "cpath" ? m_completer : nullptr);
}
