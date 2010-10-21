// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QLineEdit>
#include <QHBoxLayout>

#include "GUI/Client/UI/GraphicalString.hpp"

using namespace CF::GUI::ClientUI;

GraphicalString::GraphicalString(CF::Common::Option::ConstPtr opt, QWidget * parent)
  : GraphicalValue(parent)
{
  m_lineEdit = new QLineEdit(this);

  m_layout->addWidget(m_lineEdit);

  if(opt.get() != nullptr)
    this->setValue(opt->value<std::string>().c_str());

  connect(m_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(textUpdated(QString)));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GraphicalString::~GraphicalString()
{
  delete m_lineEdit;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalString::setValue(const QVariant & value)
{
  m_originalValue = value;
  m_lineEdit->setText(value.toString());
  return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalString::getValue() const
{
  return m_lineEdit->text();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalString::textUpdated(const QString & text)
{
  emit valueChanged();
}
