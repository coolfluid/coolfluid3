// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QLineEdit>
#include <QHBoxLayout>

#include "UI/Graphics/GraphicalString.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

GraphicalString::GraphicalString(const QString & value, QWidget * parent)
  : GraphicalValue(parent)
{
  m_line_edit = new QLineEdit(this);

  m_layout->addWidget(m_line_edit);

  this->set_value(value);

  connect(m_line_edit, SIGNAL(textChanged(QString)), this, SLOT(text_updated(QString))); }

//////////////////////////////////////////////////////////////////////////

GraphicalString::~GraphicalString()
{
  delete m_line_edit;
}

//////////////////////////////////////////////////////////////////////////

bool GraphicalString::set_value(const QVariant & value)
{
  m_original_value = value;
  m_line_edit->setText(value.toString());
  return true;
}

//////////////////////////////////////////////////////////////////////////

QVariant GraphicalString::value() const
{
  return m_line_edit->text();
}

//////////////////////////////////////////////////////////////////////////

void GraphicalString::text_updated(const QString & text)
{
  emit value_changed();
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
