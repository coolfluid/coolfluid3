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

#include "common/URI.hpp"

#include "ui/core/TreeThread.hpp"

#include "ui/graphics/GraphicalUriArray.hpp"
#include "ui/graphics/BrowserDialog.hpp"
#include "ui/graphics/SelectPathDialog.hpp"

#include <QApplication>

using namespace cf3::common;
using namespace cf3::ui::core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

GraphicalUriArray::GraphicalUriArray(const QString& sep, QWidget * parent)
  : GraphicalValue(parent)
{
  m_group_box = new QGroupBox();
  m_edit_add = new QLineEdit(m_group_box);
  m_model = new QStringListModel(m_group_box);
  m_list_view = new QListView(m_group_box);// <==== bad?
  m_bt_add = new QPushButton("+", m_group_box);
  m_bt_remove = new QPushButton("-", m_group_box);
  m_bt_up = new QPushButton("Up", m_group_box);
  m_bt_down = new QPushButton("Down", m_group_box);
  m_combo_type = new QComboBox(m_group_box);
  m_separator = sep;
  m_buttons_layout = new QVBoxLayout();
  m_box_layout = new QGridLayout(m_group_box);
  m_list_view->setModel(m_model);
  m_list_view->setSelectionMode(QAbstractItemView::ExtendedSelection);


//  m_listView->setDragDropMode(QAbstractItemView::InternalMove);
//  m_listView->setDragEnabled(true);
//  m_listView->setAcceptDrops(true);

  set_protocols(std::vector<std::string>()); // <==== bad?
  m_buttons_layout->addWidget(m_bt_up);
  m_buttons_layout->addWidget(m_bt_down);
  m_box_layout->addWidget(m_combo_type, 0, 0);
  m_box_layout->addWidget(m_edit_add, 0, 1);
  m_box_layout->addWidget(m_bt_add, 0, 2);
  m_box_layout->addWidget(m_bt_remove, 0, 3);
  m_box_layout->addWidget(m_list_view, 1, 0, 1, 4);
  m_box_layout->addLayout(m_buttons_layout, 1, 4);
  m_layout->addWidget(m_group_box);
  selection_changed(QItemSelection(), QItemSelection());

  connect(m_bt_add, SIGNAL(clicked()), this, SLOT(bt_add_clicked()));
  connect(m_bt_remove, SIGNAL(clicked()), this, SLOT(bt_remove_clicked()));
  connect(m_combo_type, SIGNAL(activated(QString)), this, SLOT(scheme_changed(QString)));
  connect(m_bt_up, SIGNAL(clicked()), this, SLOT(move_up()));
  connect(m_bt_down, SIGNAL(clicked()), this, SLOT(move_down()));
  connect(m_list_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(selection_changed(QItemSelection,QItemSelection)));

}

////////////////////////////////////////////////////////////////////////////

GraphicalUriArray::~GraphicalUriArray()
{
  delete m_edit_add;
  delete m_model;
  delete m_list_view;
  delete m_bt_add;
  delete m_bt_remove;
  delete m_buttons_layout;
  delete m_group_box;
}

////////////////////////////////////////////////////////////////////////////

void GraphicalUriArray::set_protocols(const std::vector<std::string> & protocols)
{
  m_combo_type->clear();

  if(protocols.empty())
  {
    m_combo_type->addItem("cpath");
    m_combo_type->addItem("file");
    m_combo_type->addItem("http");
  }
  else
  {
    std::vector<std::string>::const_iterator it;
    for(it = protocols.begin() ; it != protocols.end() ; it++)
      m_combo_type->addItem(it->c_str());
  }

  scheme_changed(m_combo_type->currentText());
}

////////////////////////////////////////////////////////////////////////////

bool GraphicalUriArray::set_value(const QVariant & value)
{
  QStringList list;
  bool success = false;

  if(value.type() == QVariant::String)
  {
    list = value.toString().split(m_separator);
    list.removeAll(QString());
    m_original_value = list;
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

    m_original_value = values;

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

void GraphicalUriArray::bt_add_clicked()
{

  if(m_edit_add->text().isEmpty())
  {
    if(m_combo_type->currentText() == "cpath")
    {
      SelectPathDialog spd;
      QString modified_path = m_edit_add->text();

      URI path = spd.show(modified_path.toStdString());

      if(!path.empty())
        m_edit_add->setText( path.string().c_str() );
    }
    else if(m_combo_type->currentText() == "file")
    {
      BrowserDialog nro;
      QVariant val;

      if( nro.show(true, val) )
      {
        QStringList file_list = val.toStringList();
        QStringList::iterator file = file_list.begin();

        for( ; file != file_list.end() ; ++ file)
          file->prepend( m_combo_type->currentText() + ':' );

        if(!file_list.isEmpty())
          m_model->setStringList( m_model->stringList() << file_list );
      }
    }
  }

  if(!m_edit_add->text().isEmpty())
  {
    QString pathStr = m_edit_add->text();

    if( !pathStr.startsWith(m_combo_type->currentText()) )
      pathStr.prepend(m_combo_type->currentText() + ':');

    m_model->setStringList( m_model->stringList() << pathStr );
    m_edit_add->clear();
  }

  emit value_changed();
}

////////////////////////////////////////////////////////////////////////////

void GraphicalUriArray::bt_remove_clicked()
{
  QModelIndexList selectedItems;

  selectedItems = m_list_view->selectionModel()->selectedIndexes();

  for(int i = selectedItems.size() - 1 ; i >= 0 ; i--)
  {
    QModelIndex index = selectedItems.at(i);

    m_model->removeRow(index.row(), index.parent());
  }

  emit value_changed();
}

////////////////////////////////////////////////////////////////////////////

void GraphicalUriArray::scheme_changed(const QString & type)
{
//  m_btBrowse->setVisible(type == "cpath" || type == "file");
  m_edit_add->setVisible(!type.isEmpty());
}

////////////////////////////////////////////////////////////////////////////

void GraphicalUriArray::move_up()
{
  move_items(-1);
}

////////////////////////////////////////////////////////////////////////////

void GraphicalUriArray::move_down()
{
  move_items(1);
}

////////////////////////////////////////////////////////////////////////////

void GraphicalUriArray::move_items( int step )
{
  QModelIndexList selected = m_list_view->selectionModel()->selectedRows();
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
    m_list_view->selectionModel()->select( m_model->index(*itNewSelect), QItemSelectionModel::Select );

  // update buttons enabled states
  selection_changed(QItemSelection(), QItemSelection());
}

////////////////////////////////////////////////////////////////////////////

void GraphicalUriArray::selection_changed(const QItemSelection &, const QItemSelection & )
{
  QModelIndexList selected = m_list_view->selectionModel()->selectedRows();

  // we enable the "Up" button if exactly one item is selected, but not the first one
  m_bt_up->setEnabled( selected.size() == 1 && selected.first().row() != 0 );

  // we enable the "Down" button if exactly one item is selected, but not the last one
  m_bt_down->setEnabled( selected.size() == 1 && selected.last().row() != m_model->rowCount() - 1 );

}

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
