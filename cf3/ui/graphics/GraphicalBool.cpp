// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QCheckBox>
#include <QVBoxLayout>

#include "common/StringConversion.hpp"

#include "ui/graphics/GraphicalBool.hpp"

using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

GraphicalBool::GraphicalBool(bool value, QWidget * parent)
  : GraphicalValue(parent)
{
  m_check_box = new QCheckBox(this);

  m_layout->addWidget(m_check_box);

  this->set_value(value);

  connect(m_check_box, SIGNAL(stateChanged(int)), this, SLOT(state_changed(int)));
}

//////////////////////////////////////////////////////////////////////////

GraphicalBool::~GraphicalBool()
{
  delete m_check_box;
}

//////////////////////////////////////////////////////////////////////////

bool GraphicalBool::set_value(const QVariant & value)
{
  bool valid = true;

  if(value.type() == QVariant::Bool)
    m_check_box->setChecked(value.toBool());
  else if(value.type() == QVariant::String)
    m_check_box->setChecked( from_str<bool>( value.toString().toStdString() ));
  else
    valid = false;

  if(valid)
    m_original_value = value;

  return valid;
}

//////////////////////////////////////////////////////////////////////////

QVariant GraphicalBool::value() const
{
  return m_check_box->isChecked();
}

//////////////////////////////////////////////////////////////////////////

void GraphicalBool::state_changed(int state)
{
  emit value_changed();
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
