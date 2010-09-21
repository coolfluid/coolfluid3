// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QHBoxLayout>
#include <QDebug>

#include "GUI/Client/GraphicalValue.hpp"

using namespace CF::GUI::Client;

GraphicalValue::GraphicalValue(QWidget *parent) :
    QWidget(parent),
    m_committing(false)
{
  this->m_layout = new QHBoxLayout(this);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString GraphicalValue::getValueString() const
{
  QVariant value = this->getValue();

  if(value.type() == QVariant::StringList)
    return value.toStringList().join(":");

  return value.toString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalValue::getOriginalValue() const
{
  return m_originalValue;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString GraphicalValue::getOriginalString() const
{
  if(m_originalValue.type() == QVariant::StringList)
    return m_originalValue.toStringList().join(":");

  return m_originalValue.toString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalValue::isModified() const
{
  return getOriginalString() != getValueString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalValue::commit()
{
  m_originalValue = getValue();
  emit valueChanged();
}
