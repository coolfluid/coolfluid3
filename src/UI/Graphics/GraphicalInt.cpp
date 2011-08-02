// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDoubleSpinBox>
#include <QVariant>
#include <QVBoxLayout>

#include "Math/Consts.hpp"

#include "UI/Graphics/GraphicalInt.hpp"

using namespace CF::Math;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

GraphicalInt::GraphicalInt(bool isUint, QVariant value, QWidget * parent)
  : GraphicalValue(parent),
    m_isUint(isUint)
{
  m_spinBox = new QDoubleSpinBox(/*isUint, */this);

  if(m_isUint)
    m_spinBox->setRange(Consts::uint_min(), Consts::uint_max());
  else
    m_spinBox->setRange(Consts::int_min(), Consts::int_max());

  m_layout->addWidget(m_spinBox);

  m_spinBox->setDecimals(0);

  if(isUint)
    this->setValue(value.toUInt());
  else
    this->setValue(value.toInt());

  connect(m_spinBox, SIGNAL(valueChanged(double)), this, SLOT(integerChanged(double)));
}

////////////////////////////////////////////////////////////////////////////

GraphicalInt::~GraphicalInt()
{
  delete m_spinBox;
}

////////////////////////////////////////////////////////////////////////////

bool GraphicalInt::setValue(const QVariant & value)
{
  if(value.type() == (m_isUint ? QVariant::UInt : QVariant::Int))
  {
    m_originalValue = value;
    m_spinBox->setValue(value.toDouble());
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////

QVariant GraphicalInt::value() const
{
  if(m_isUint)
    return (Uint) m_spinBox->value();

  return (int) m_spinBox->value();
}

////////////////////////////////////////////////////////////////////////////

void GraphicalInt::integerChanged(double value)
{
  emit valueChanged();
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF
