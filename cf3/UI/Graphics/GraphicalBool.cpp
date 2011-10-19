// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QCheckBox>
#include <QVBoxLayout>

#include "common/StringConversion.hpp"

#include "UI/Graphics/GraphicalBool.hpp"

using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

GraphicalBool::GraphicalBool(bool value, QWidget * parent)
  : GraphicalValue(parent)
{
  m_checkBox = new QCheckBox(this);

  m_layout->addWidget(m_checkBox);

  this->setValue(value);

  connect(m_checkBox, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
}

//////////////////////////////////////////////////////////////////////////

GraphicalBool::~GraphicalBool()
{
  delete m_checkBox;
}

//////////////////////////////////////////////////////////////////////////

bool GraphicalBool::setValue(const QVariant & value)
{
  bool valid = true;

  if(value.type() == QVariant::Bool)
    m_checkBox->setChecked(value.toBool());
  else if(value.type() == QVariant::String)
    m_checkBox->setChecked( from_str<bool>( value.toString().toStdString() ));
  else
    valid = false;

  if(valid)
    m_originalValue = value;

  return valid;
}

//////////////////////////////////////////////////////////////////////////

QVariant GraphicalBool::value() const
{
  return m_checkBox->isChecked();
}

//////////////////////////////////////////////////////////////////////////

void GraphicalBool::stateChanged(int state)
{
  emit valueChanged();
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
