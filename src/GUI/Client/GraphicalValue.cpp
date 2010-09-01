#include <QHBoxLayout>

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
  return m_originalValue.toString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalValue::isModified() const
{
  return m_originalValue != this->getValue();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalValue::commit()
{
  m_committing = true,
  this->setValue(m_originalValue);
  m_committing = false;
}
