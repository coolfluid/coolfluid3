#include <QLineEdit>
#include <QHBoxLayout>

#include "GUI/Client/GraphicalString.hpp"

using namespace CF::GUI::Client;

GraphicalString::GraphicalString(QWidget * parent)
  : GraphicalValue(parent)
{
  m_lineEdit = new QLineEdit(this);

  m_layout->addWidget(m_lineEdit);

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
