// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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

#include "UI/Core/NLog.hpp"

#include "UI/Graphics/GraphicalArray.hpp"

using namespace CF::UI::Core;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

GraphicalArray::GraphicalArray(QValidator * validator, QWidget * parent)
  : GraphicalValue(parent)
{
  m_groupBox = new QGroupBox(this);
  m_editAdd = new QLineEdit(this);
  m_model = new QStringListModel(this);
  m_listView = new QListView(this);
  m_btRemove = new QPushButton("Remove", this);

  m_boxLayout = new QGridLayout(m_groupBox);

  m_editAdd->setValidator(validator);

  m_listView->setModel(m_model);
  m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

  m_boxLayout->addWidget(m_editAdd, 0, 0);
  m_boxLayout->addWidget(m_listView, 1, 0);
  m_boxLayout->addWidget(m_btRemove, 1, 1);

  m_layout->addWidget(m_groupBox);

  connect(m_btRemove, SIGNAL(clicked()), this, SLOT(btRemoveClicked()));
}

//////////////////////////////////////////////////////////////////////////

GraphicalArray::~GraphicalArray()
{
  delete m_editAdd;
  delete m_model;
  delete m_listView;
  delete m_btRemove;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalArray::setValidator(QValidator * validator)
{
  m_editAdd->setValidator(validator);
}

//////////////////////////////////////////////////////////////////////////

bool GraphicalArray::setValue(const QVariant & value)
{
  QStringList invalidValues;
  QStringList list;
  bool success;
  const QValidator * validator = m_editAdd->validator();
  int pos;

  if(value.type() == QVariant::String)
  {
    QString valueStr = value.toString();

    bool valid = validator == nullptr || validator->validate(valueStr, pos) == QValidator::Acceptable;

    if(!valid)
      invalidValues << valueStr;
    else
    {
      list = value.toString().split("@@");
      list.removeAll(QString());
//      m_model->setStringList(list);
    }

  }
  else if(value.type() == QVariant::StringList)
  {
    QStringList values = value.toStringList();
    QStringList::iterator it = values.begin();

    success = true;

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
  }

  success = invalidValues.empty();

  list.removeDuplicates();
  m_originalValue = list;
  m_model->setStringList(list);

  if(!success)
  {
    QString msg;

    if(invalidValues.count() == 1)
      msg = "The following value is not valid: %1";
    else
      msg = "The following values are not valid: %1";

    msg = msg.arg(invalidValues.join("\"\n   \"").prepend("\n   \"").append("\""));

    NLog::globalLog()->addMessage(msg);
  }

  return success;
}

//////////////////////////////////////////////////////////////////////////

QVariant GraphicalArray::value() const
{
  return m_model->stringList();
}

//////////////////////////////////////////////////////////////////////////

void GraphicalArray::btRemoveClicked()
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

//////////////////////////////////////////////////////////////////////////

void GraphicalArray::keyPressEvent(QKeyEvent * event)
{
  GraphicalValue::keyPressEvent(event);

  // if the path line edit has the focus
  if(m_editAdd->hasFocus())
  {
    // key code for the pressed key
    int pressedKey = event->key();

    // Qt::Key_Enter : enter key located on the keypad
    // Qt::Key_Return : return key
    if(pressedKey == Qt::Key_Enter || pressedKey == Qt::Key_Return)
    {
      m_model->setStringList( m_model->stringList() << m_editAdd->text() );

      m_editAdd->clear();

      emit valueChanged();
    }
  }
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF
