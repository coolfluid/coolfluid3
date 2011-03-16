// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QComboBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QStringListModel>
#include <QValidator>
#include <QVBoxLayout>

#include "Common/URI.hpp"

#include "GUI/Client/Core/TreeThread.hpp"

#include "GUI/Client/UI/GraphicalUriArray.hpp"
#include "GUI/Client/UI/NRemoteOpen.hpp"
#include "GUI/Client/UI/SelectPathDialog.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientUI;

GraphicalUriArray::GraphicalUriArray(QWidget * parent)
  : GraphicalValue(parent)
{
  m_editAdd = new QLineEdit(this);
  m_model = new QStringListModel(this);
  m_listView = new QListView(this);
  m_btAdd = new QPushButton("+", this);
  m_btRemove = new QPushButton("-", this);
  m_comboType = new QComboBox(this);

  m_buttonsLayout = new QVBoxLayout();
  m_leftLayout = new QVBoxLayout();
  m_topLayout = new QHBoxLayout();

  m_listView->setModel(m_model);
  m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

//  m_btAdd->setFlat(true);
//  m_btRemove->setFlat(true);

  setProtocols(std::vector<std::string>());

  m_topLayout->addWidget(m_comboType);
  m_topLayout->addWidget(m_editAdd);
  m_topLayout->addWidget(m_btAdd);
  m_topLayout->addWidget(m_btRemove);

  m_leftLayout->addLayout(m_topLayout);
  m_leftLayout->addWidget(m_listView);

  m_layout->addLayout(m_leftLayout);
  m_layout->addLayout(m_buttonsLayout);

  connect(m_btAdd, SIGNAL(clicked()), this, SLOT(btAddClicked()));
  connect(m_btRemove, SIGNAL(clicked()), this, SLOT(btRemoveClicked()));
  connect(m_comboType, SIGNAL(activated(QString)), this, SLOT(changeType(QString)));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GraphicalUriArray::~GraphicalUriArray()
{
  delete m_editAdd;
  delete m_model;
  delete m_listView;
  delete m_btAdd;
  delete m_btRemove;
  delete m_buttonsLayout;
  delete m_leftLayout;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalUriArray::setProtocols(const std::vector<std::string> & protocols)
{
  m_comboType->clear();

  if(protocols.empty())
  {
    m_comboType->addItem("cpath");
    m_comboType->addItem("file");
    m_comboType->addItem("http");
  }
  else
  {
    std::vector<std::string>::const_iterator it;
    for(it = protocols.begin() ; it != protocols.end() ; it++)
      m_comboType->addItem(it->c_str());
  }

  changeType(m_comboType->currentText());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalUriArray::setValue(const QVariant & value)
{
  QStringList list;
  bool success = false;

  if(value.type() == QVariant::String)
  {
    list = value.toString().split("@@");
    list.removeAll(QString());
    m_originalValue = list;
    m_model->setStringList(list);
    success = true;
  }
  else if(value.type() == QVariant::StringList)
  {
    QStringList values = value.toStringList();
    QStringList::iterator it = values.begin();

    success = true;

    for( ; it != values.end() ; it++)
    {
      if( !it->isEmpty() )
        list << *it;
    }

    m_originalValue = values;

    list.removeDuplicates();
    m_model->setStringList(list);
  }

  return success;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalUriArray::value() const
{
  return m_model->stringList();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalUriArray::btAddClicked()
{

  if(m_editAdd->text().isEmpty())
  {
    if(m_comboType->currentText() == "cpath")
    {
      SelectPathDialog spd;
      QString modified_path = m_editAdd->text();

      URI path = spd.show(modified_path.toStdString());

      if(!path.empty())
        m_editAdd->setText( path.string().c_str() );
    }
    else if(m_comboType->currentText() == "file")
    {
      NRemoteOpen::Ptr nro = NRemoteOpen::create();
      bool canceled;

      QString filename = nro->show("", &canceled);

      if(!canceled && !filename.isEmpty())
      {
        m_editAdd->setText(filename);
      }
    }
  }

  if(!m_editAdd->text().isEmpty())
  {
    QString pathStr = m_editAdd->text();

    if( !pathStr.startsWith(m_comboType->currentText()) )
      pathStr.prepend(m_comboType->currentText() + ':');

    m_model->setStringList( m_model->stringList() << pathStr );
    m_editAdd->clear();
  }

  emit valueChanged();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalUriArray::btRemoveClicked()
{
  QModelIndexList selectedItems;

  selectedItems = m_listView->selectionModel()->selectedIndexes();

  for(int i = selectedItems.size() - 1 ; i >= 0 ; i--)
  {
    QModelIndex index = selectedItems.at(i);

    m_model->removeRow(index.row(), index.parent());
  }

  emit valueChanged();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalUriArray::changeType(const QString & type)
{
//  m_btBrowse->setVisible(type == "cpath" || type == "file");
  m_editAdd->setVisible(!type.isEmpty());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalUriArray::btBrowseClicked()
{
}
