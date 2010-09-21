// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QHBoxLayout>
#include <QListView>
#include <QPushButton>
#include <QStringListModel>
#include <QVBoxLayout>

#include "GUI/Client/ClientRoot.hpp"

#include "GUI/Client/GraphicalUrlArray.hpp"

using namespace CF::GUI::Client;

GraphicalUrlArray::GraphicalUrlArray(QWidget * parent)
  : GraphicalValue(parent)
{
  m_model = new QStringListModel(this);
  m_listView = new QListView(this);
  m_btAdd = new QPushButton("Add", this);
  m_btRemove = new QPushButton("Remove", this);
  m_browser = NRemoteOpen::create();
  m_buttonsLayout = new QVBoxLayout();

  m_listView->setModel(m_model);
  m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

  m_buttonsLayout->addWidget(m_btAdd);
  m_buttonsLayout->addWidget(m_btRemove);

  m_layout->addWidget(m_listView);
  m_layout->addLayout(m_buttonsLayout);

  connect(m_btAdd, SIGNAL(clicked()), this, SLOT(btAddClicked()));
  connect(m_btRemove, SIGNAL(clicked()), this, SLOT(btRemoveClicked()));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GraphicalUrlArray::~GraphicalUrlArray()
{
  delete m_listView;
  delete m_model;
  delete m_btAdd;
  delete m_btRemove;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalUrlArray::getValue() const
{
  return m_model->stringList();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalUrlArray::setValue(const QVariant & value)
{
  if(value.type() == QVariant::StringList)
  {
    m_originalValue = value;
    m_model->setStringList(value.toStringList());
    return true;
  }
  else if(value.type() == QVariant::String)
  {
    QStringList list = value.toString().split(":");
    m_originalValue = list;
    m_model->setStringList(list);
    return true;
  }

  return false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalUrlArray::btAddClicked()
{
  QStringList files = m_browser->showMultipleSelect();
  QStringList currentFilesList = m_model->stringList();

  QStringList::iterator it = files.begin();

  while(it != files.end())
  {
    QString & filename = *it;

    // if the file is not already in the list
    if(!filename.isEmpty() && !currentFilesList.contains(filename))
      currentFilesList << filename;

    it++;
  }

  m_model->setStringList(currentFilesList);

  if(!files.isEmpty())
    emit valueChanged();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalUrlArray::btRemoveClicked()
{
  QModelIndexList selectedItems;

  selectedItems = m_listView->selectionModel()->selectedIndexes();

  for(int i = selectedItems.size() - 1 ; i >= 0 ; i--)
  {
    QModelIndex index = selectedItems.at(i);

    m_model->removeRow(index.row(), index.parent());
  }
}
