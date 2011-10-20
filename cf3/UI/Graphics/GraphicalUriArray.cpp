// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QStringListModel>
#include <QValidator>
#include <QVBoxLayout>

#include <QDebug>

#include "common/URI.hpp"

#include "UI/Core/TreeThread.hpp"

#include "UI/Graphics/GraphicalUriArray.hpp"
#include "UI/Graphics/NRemoteOpen.hpp"
#include "UI/Graphics/SelectPathDialog.hpp"

using namespace cf3::common;
using namespace cf3::UI::Core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

GraphicalUriArray::GraphicalUriArray(const QString& sep, QWidget * parent)
  : GraphicalValue(parent)
{
  m_groupBox = new QGroupBox(parent);
  m_editAdd = new QLineEdit(m_groupBox);
  m_model = new QStringListModel(m_groupBox);
  m_listView = new QListView(m_groupBox);
  m_btAdd = new QPushButton("+", m_groupBox);
  m_btRemove = new QPushButton("-", m_groupBox);
  m_btUp = new QPushButton("Up", m_groupBox);
  m_btDown = new QPushButton("Down", m_groupBox);
  m_comboType = new QComboBox(m_groupBox);
  m_separator = sep;

  m_buttonsLayout = new QVBoxLayout();
  m_boxLayout = new QGridLayout(m_groupBox);

  m_listView->setModel(m_model);
  m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
//  m_listView->setDragDropMode(QAbstractItemView::InternalMove);
//  m_listView->setDragEnabled(true);
//  m_listView->setAcceptDrops(true);

  setProtocols(std::vector<std::string>());

  m_buttonsLayout->addWidget(m_btUp);
  m_buttonsLayout->addWidget(m_btDown);

  m_boxLayout->addWidget(m_comboType, 0, 0);
  m_boxLayout->addWidget(m_editAdd, 0, 1);
  m_boxLayout->addWidget(m_btAdd, 0, 2);
  m_boxLayout->addWidget(m_btRemove, 0, 3);
  m_boxLayout->addWidget(m_listView, 1, 0, 1, 4);
  m_boxLayout->addLayout(m_buttonsLayout, 1, 4);

  m_layout->addWidget(m_groupBox);

  selectionChanged(QItemSelection(), QItemSelection());

  connect(m_btAdd, SIGNAL(clicked()), this, SLOT(btAddClicked()));
  connect(m_btRemove, SIGNAL(clicked()), this, SLOT(btRemoveClicked()));
  connect(m_comboType, SIGNAL(activated(QString)), this, SLOT(changeType(QString)));
  connect(m_btUp, SIGNAL(clicked()), this, SLOT(moveUp()));
  connect(m_btDown, SIGNAL(clicked()), this, SLOT(moveDown()));
  connect(m_listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
}

////////////////////////////////////////////////////////////////////////////

GraphicalUriArray::~GraphicalUriArray()
{
  delete m_editAdd;
  delete m_model;
  delete m_listView;
  delete m_btAdd;
  delete m_btRemove;
  delete m_buttonsLayout;
  delete m_groupBox;
}

////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////

bool GraphicalUriArray::setValue(const QVariant & value)
{
  QStringList list;
  bool success = false;

  if(value.type() == QVariant::String)
  {
    list = value.toString().split(m_separator);
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

////////////////////////////////////////////////////////////////////////////

QVariant GraphicalUriArray::value() const
{
  return m_model->stringList();
}

////////////////////////////////////////////////////////////////////////////

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

      QStringList fileList = nro->showMultipleSelect("");
      QStringList::iterator file = fileList.begin();

      for( ; file != fileList.end() ; ++ file)
      {
        file->prepend( m_comboType->currentText() + ':' );
      }

      if(!fileList.isEmpty())
        m_model->setStringList( m_model->stringList() << fileList );
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

////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////

void GraphicalUriArray::changeType(const QString & type)
{
//  m_btBrowse->setVisible(type == "cpath" || type == "file");
  m_editAdd->setVisible(!type.isEmpty());
}

////////////////////////////////////////////////////////////////////////////

void GraphicalUriArray::moveUp()
{
  moveItems(-1);
}

////////////////////////////////////////////////////////////////////////////

void GraphicalUriArray::moveDown()
{
  moveItems(1);
//  QModelIndexList selected = m_listView->selectionModel()->selectedIndexes();
//  QModelIndexList::iterator it = selected.begin();
//  QStringList items = m_model->stringList();
//  QList<int> newSelection;

//  QList<int> selection;

//  // move the elements
//  for( ; it != selected.end() ; ++it )
//    selection.append( it->row() );

//  qSort(selection);

//  for(int row = 0 ; row < selected.size() ; ++row )
//  {
//    int newRow = row + 1;

////      if(  newRow >= 0 && newRow < items.count() )
//      qDebug() << row << newRow;

//      items.move( row, newRow );    // move the row
//      newSelection.append( newRow );
//  }

//  m_model->setStringList(items); // set the new items

//  // select items that were  just moved
//  QList<int>::iterator itNewSelect = newSelection.begin();
//  for( ; itNewSelect != newSelection.end(); ++itNewSelect)
//    m_listView->selectionModel()->select( m_model->index(*(itNewSelect)), QItemSelectionModel::Select );

//  // update buttons enabled states
//  selectionChanged(QItemSelection(), QItemSelection());

//  qDebug() << "===========================";

}

////////////////////////////////////////////////////////////////////////////

void GraphicalUriArray::moveItems( int step )
{
  QModelIndexList selected = m_listView->selectionModel()->selectedRows();
  QModelIndexList::iterator it = selected.begin();
  QStringList items = m_model->stringList();
  QList<int> newSelection;

  // move the elements
  for( ; it != selected.end() ; ++it )
  {
      int row = it->row();
      int newRow = row + step;

//      if(  newRow >= 0 && newRow < items.count() )
      items.move( row, newRow );    // move the row
      newSelection.append( newRow );
  }

  m_model->setStringList(items); // set the new items

  // select items that were just moved
  QList<int>::iterator itNewSelect = newSelection.begin();
  for( ; itNewSelect != newSelection.end(); ++ itNewSelect)
    m_listView->selectionModel()->select( m_model->index(*itNewSelect), QItemSelectionModel::Select );

  // update buttons enabled states
  selectionChanged(QItemSelection(), QItemSelection());
}

////////////////////////////////////////////////////////////////////////////

void GraphicalUriArray::selectionChanged(const QItemSelection &, const QItemSelection & )
{
  QModelIndexList selected = m_listView->selectionModel()->selectedRows();

  // we enable the "Up" button if exactly one item is selected, but not the first one
  m_btUp->setEnabled( selected.size() == 1 && selected.first().row() != 0 );

  // we enable the "Down" button if exactly one item is selected, but not the last one
  m_btDown->setEnabled( selected.size() == 1 && selected.last().row() != m_model->rowCount() - 1 );

}

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
