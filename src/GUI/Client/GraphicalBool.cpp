#include <QCheckBox>
#include <QVBoxLayout>

#include "GUI/Client/GraphicalBool.hpp"

using namespace CF::GUI::Client;

GraphicalBool::GraphicalBool(QWidget * parent)
  : GraphicalValue(parent)
{
  m_checkBox = new QCheckBox(this);

  m_layout->addWidget(m_checkBox);
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
