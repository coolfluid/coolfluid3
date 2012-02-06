// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QGroupBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QStringListModel>
#include <QValidator>
#include <QVBoxLayout>

#include "ui/core/NLog.hpp"

#include "ui/graphics/GraphicalArray.hpp"

using namespace cf3::ui::core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

GraphicalArray::GraphicalArray(QValidator * validator, const QString& sep, QWidget * parent)
  : GraphicalValue(parent)
{
  m_group_box = new QGroupBox(this);
  m_edit_add = new QLineEdit(this);
  m_model = new QStringListModel(this);
  m_list_view = new QListView(this);
  m_bt_remove = new QPushButton("Remove", this);
  m_separator = sep;

  m_box_layout = new QGridLayout(m_group_box);

  m_edit_add->setValidator(validator);

  m_list_view->setModel(m_model);
  m_list_view->setSelectionMode(QAbstractItemView::ExtendedSelection);

  m_box_layout->addWidget(m_edit_add, 0, 0);
  m_box_layout->addWidget(m_list_view, 1, 0);
  m_box_layout->addWidget(m_bt_remove, 1, 1);

  m_layout->addWidget(m_group_box);

  connect(m_bt_remove, SIGNAL(clicked()), this, SLOT(bt_remove_clicked()));
}

//////////////////////////////////////////////////////////////////////////

GraphicalArray::~GraphicalArray()
{
  delete m_edit_add;
  delete m_model;
  delete m_list_view;
  delete m_bt_remove;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalArray::set_validator(QValidator * validator)
{
  m_edit_add->setValidator(validator);
}

//////////////////////////////////////////////////////////////////////////

bool GraphicalArray::set_value(const QVariant & value)
{
  QStringList invalidValues;
  QStringList values;
  QStringList list;
  bool success = true;
  const QValidator * validator = m_edit_add->validator();
  int pos;

  if(value.type() == QVariant::String)
    values = value.toString().split(m_separator);
  else if(value.type() == QVariant::StringList)
    values = value.toStringList();
  else
    success = false;

  if( success )
  {
    QStringList::iterator it = values.begin();

    for( ; it != values.end() ; it++)
    {
      QString value = *it;

        bool valid = validator == nullptr ||
                     value.isEmpty() ||
                     validator->validate(value, pos) == QValidator::Acceptable;

        if(!valid)
          invalidValues << value;
        else
          list << value;

        success &= valid;
    }

    success = invalidValues.empty();
  }

  if(!success)
  {
    QString msg;

    if(invalidValues.count() == 1)
      msg = "The following value is not valid: %1";
    else
      msg = "The following values are not valid: %1";

    msg = msg.arg(invalidValues.join("\"\n   \"").prepend("\n   \"").append("\""));

    NLog::global()->add_message(msg);
  }
  else
  {
    m_original_value = list;
    m_model->setStringList(list);
    emit value_changed();
  }

  return success;
}

//////////////////////////////////////////////////////////////////////////

QVariant GraphicalArray::value() const
{
  return m_model->stringList();
}

//////////////////////////////////////////////////////////////////////////

void GraphicalArray::bt_remove_clicked()
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

//////////////////////////////////////////////////////////////////////////

void GraphicalArray::keyPressEvent(QKeyEvent * event)
{
  GraphicalValue::keyPressEvent(event);

  // if the path line edit has the focus
  if(m_edit_add->hasFocus())
  {
    // key code for the pressed key
    int pressedKey = event->key();

    // Qt::Key_Enter : enter key located on the keypad
    // Qt::Key_Return : return key
    if(pressedKey == Qt::Key_Enter || pressedKey == Qt::Key_Return)
    {
      m_model->setStringList( m_model->stringList() << m_edit_add->text() );

      m_edit_add->clear();

      emit value_changed();
    }
  }
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
