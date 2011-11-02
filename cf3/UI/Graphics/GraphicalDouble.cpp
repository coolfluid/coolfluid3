// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDoubleValidator>
#include <QLineEdit>
#include <QVariant>
#include <QVBoxLayout>

#include "UI/Graphics/GraphicalDouble.hpp"

using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

GraphicalDouble::GraphicalDouble(Real value, QWidget * parent)
  : GraphicalValue(parent)
{
  m_line_edit = new QLineEdit(this);
  m_validator = new QDoubleValidator(this);

  m_validator->setNotation(QDoubleValidator::ScientificNotation);
  m_line_edit->setValidator(m_validator);

  m_layout->addWidget(m_line_edit);

  this->set_value(value);

  connect(m_line_edit, SIGNAL(textChanged(QString)), this, SLOT(text_updated(QString)));
}

//////////////////////////////////////////////////////////////////////////

GraphicalDouble::~GraphicalDouble()
{
  delete m_line_edit;
  delete m_validator;
}

//////////////////////////////////////////////////////////////////////////

bool GraphicalDouble::set_value(const QVariant & value)
{
  QString valueString = value.toString();
  int pos;

  // if it is a double or a string that represents a double value
  if(value.canConvert(QVariant::Double) &&
     m_validator->validate(valueString, pos) == QValidator::Acceptable)
  {
    m_original_value = value;
    m_line_edit->setText(valueString);
    return true;
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////

QVariant GraphicalDouble::value() const
{
  return m_line_edit->text().toDouble();
}

//////////////////////////////////////////////////////////////////////////

void GraphicalDouble::text_updated(const QString & text)
{
  emit value_changed();
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
