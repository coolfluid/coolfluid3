// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDoubleValidator>
#include <QLineEdit>
#include <QVariant>
#include <QVBoxLayout>

#include "GUI/Client/GraphicalDouble.hpp"

using namespace CF::GUI::Client;

GraphicalDouble::GraphicalDouble(QWidget * parent)
  : GraphicalValue(parent)
{
  m_lineEdit = new QLineEdit(this);
  m_validator = new QDoubleValidator(this);

  m_validator->setNotation(QDoubleValidator::ScientificNotation);
  m_lineEdit->setValidator(m_validator);

  m_layout->addWidget(m_lineEdit);

  connect(m_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(textUpdated(QString)));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GraphicalDouble::~GraphicalDouble()
{
  delete m_lineEdit;
  delete m_validator;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalDouble::setValue(const QVariant & value)
{
  QString valueString = value.toString();
  int pos;

  // if it is a double or a string that represents a double value
  if(value.canConvert(QVariant::Double) ||
     m_validator->validate(valueString, pos) == QValidator::Acceptable)
  {
    m_originalValue = value;
    m_lineEdit->setText(valueString);
    return true;
  }

  return false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalDouble::getValue() const
{
  return m_lineEdit->text();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalDouble::textUpdated(const QString & text)
{
  emit valueChanged();
}
