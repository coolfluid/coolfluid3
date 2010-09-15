// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QCheckBox>
#include <QVBoxLayout>

#include "GUI/Client/GraphicalBool.hpp"

using namespace CF::GUI::Client;

GraphicalBool::GraphicalBool(QWidget * parent)
  : GraphicalValue(parent)
{
  m_checkBox = new QCheckBox(this);

  m_layout->addWidget(m_checkBox);

  connect(m_checkBox, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GraphicalBool::~GraphicalBool()
{
  delete m_checkBox;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalBool::setValue(const QVariant & value)
{
  if(value.type() == QVariant::Bool)
    m_checkBox->setChecked(value.toBool());
  else
    m_checkBox->setChecked(value.toString() == "true" || value.toString() == "yes");

  m_originalValue = value;

  return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalBool::getValue() const
{
  return m_checkBox->isChecked();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalBool::stateChanged(int state)
{
  emit valueChanged();
}
