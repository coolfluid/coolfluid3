// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QComboBox>
#include <QVBoxLayout>

#include "GraphicalRestrictedList.hpp"

using namespace CF::GUI::ClientUI;

GraphicalRestrictedList::GraphicalRestrictedList(QWidget * parent)
  : GraphicalValue(parent)
{
  m_comboChoices = new QComboBox(this);

  m_layout->addWidget(m_comboChoices);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GraphicalRestrictedList::~GraphicalRestrictedList()
{
  delete m_comboChoices;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalRestrictedList::setValue(const QVariant & value)
{
  bool valid = true;

  if(value.type() == QVariant::StringList)
  {
    m_comboChoices->clear();
    m_comboChoices->addItems(value.toStringList());
  }
  else if(value.type() == QVariant::String)
  {
    int index = m_comboChoices->findText(value.toString());

    if(index > -1)
      m_comboChoices->setCurrentIndex(index);
    else
      valid = false;
  }
  else
    valid = false;

  return valid;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalRestrictedList::getValue() const
{
  return m_comboChoices->currentText();
}
