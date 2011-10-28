// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDoubleSpinBox>
#include <QVariant>
#include <QVBoxLayout>

#include "math/Consts.hpp"

#include "UI/Graphics/GraphicalInt.hpp"

using namespace cf3::math;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

GraphicalInt::GraphicalInt(bool isUint, QVariant value, QWidget * parent)
  : GraphicalValue(parent),
    m_isUint(isUint)
{
  m_spin_box = new QDoubleSpinBox(/*isUint, */this);

  if(m_isUint)
    m_spin_box->setRange(Consts::uint_min(), Consts::uint_max());
  else
    m_spin_box->setRange(Consts::int_min(), Consts::int_max());

  m_layout->addWidget(m_spin_box);

  m_spin_box->setDecimals(0);

  if(isUint)
    this->set_value(value.toUInt());
  else
    this->set_value(value.toInt());

  connect(m_spin_box, SIGNAL(valueChanged(double)), this, SLOT(integer_changed(double)));
}

////////////////////////////////////////////////////////////////////////////

GraphicalInt::~GraphicalInt()
{
  delete m_spin_box;
}

////////////////////////////////////////////////////////////////////////////

bool GraphicalInt::set_value(const QVariant & value)
{
  if(value.type() == (m_isUint ? QVariant::UInt : QVariant::Int))
  {
    m_original_value = value;
    m_spin_box->setValue(value.toDouble());
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////

QVariant GraphicalInt::value() const
{
  if(m_isUint)
    return (Uint) m_spin_box->value();

  return (int) m_spin_box->value();
}

////////////////////////////////////////////////////////////////////////////

void GraphicalInt::integer_changed(double value)
{
  emit value_changed();
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
