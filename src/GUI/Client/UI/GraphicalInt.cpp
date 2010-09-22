// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QSpinBox>
#include <QVariant>
#include <QVBoxLayout>

#include <climits>

#include "GUI/Client/UI/GraphicalInt.hpp"

using namespace CF::GUI::Client;

GraphicalInt::GraphicalInt(bool isUint, QWidget * parent)
  : GraphicalValue(parent),
    m_isUint(isUint)
{
  m_spinBox = new QSpinBox(this);

  m_spinBox->setRange((m_isUint ? 0 : INT_MIN), INT_MAX);

  m_layout->addWidget(m_spinBox);

  connect(m_spinBox, SIGNAL(valueChanged(int)), this, SLOT(integerChanged(int)));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GraphicalInt::~GraphicalInt()
{
  delete m_spinBox;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalInt::setValue(const QVariant & value)
{
  if(value.canConvert(m_isUint ? QVariant::UInt : QVariant::Int))
  {
    m_originalValue = value;
    m_spinBox->setValue(m_isUint ? value.toUInt() : value.toInt());
    return true;
  }

  return false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalInt::getValue() const
{
  return m_spinBox->value();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalInt::integerChanged(int value)
{
  emit valueChanged();
}
