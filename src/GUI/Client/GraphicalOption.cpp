#include <QCheckBox>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QVariant>

#include <climits>

#include "Common/BasicExceptions.hpp"

#include "GUI/Client/GraphicalBool.hpp"
#include "GUI/Client/GraphicalDouble.hpp"
#include "GUI/Client/GraphicalInt.hpp"
#include "GUI/Client/GraphicalString.hpp"
#include "GUI/Client/GraphicalUrl.hpp"
#include "GUI/Client/GraphicalUrlArray.hpp"
#include "GUI/Client/GraphicalValue.hpp"

#include "GUI/Client/UnknownTypeException.hpp"

#include "GUI/Client/GraphicalOption.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

GraphicalOption::GraphicalOption(OptionType::Type type, QWidget * parent)
{
  switch(type)
  {
    // if type valueWidget is a bool
  case OptionType::TYPE_BOOL:
    m_valueWidget = new GraphicalBool(parent);
    break;

    // if type valueWidget is a string
  case OptionType::TYPE_STRING:
    m_valueWidget = new GraphicalUrl(parent);
    //m_valueWidget = new GraphicalString(parent);
    break;

    // if type valueWidget is a double
  case OptionType::TYPE_DOUBLE:
    m_valueWidget = new GraphicalDouble(parent);
    break;

    // if type valueWidget is an int
  case OptionType::TYPE_INT:
    m_valueWidget = new GraphicalInt(false, parent);
    break;

    // if type valueWidget is an unsigned int
  case OptionType::TYPE_UNSIGNED_INT:
    m_valueWidget = new GraphicalInt(true, parent);
    break;

    // if type valueWidget is a files list
  case OptionType::TYPE_FILES:
    m_valueWidget = new GraphicalUrlArray(parent);
    break;

    // if type valueWidget is a library list
  case OptionType::TYPE_LIBRARIES:
    m_valueWidget = new GraphicalUrlArray(parent);
    break;

    // if type valueWidget is a string
  case OptionType::TYPE_PATH:
    m_valueWidget = new GraphicalUrl(parent);
    break;

  default:
    throw UnknownTypeException(FromHere(), "Uknowmn option type");
  }

  m_name = new QLabel();

  m_type = type;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GraphicalOption::~GraphicalOption()
{
  delete m_name;
  delete m_valueWidget;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString GraphicalOption::getName() const
{
  return m_name->text();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalOption::setName(const QString & name)
{
  m_name->setText(name);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalOption::getValue() const
{
  return m_valueWidget->getValue();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString GraphicalOption::getValueString() const
{
  return m_valueWidget->getValueString();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

OptionType::Type GraphicalOption::getType() const
{
  return m_type;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalOption::setValue(const QVariant & newValue)
{
  m_valueWidget->setValue(newValue);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalOption::addToLayout(QFormLayout * layout)
{
  if(layout != CFNULL)
    layout->addRow(m_name, m_valueWidget);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalOption::setEnabled(bool enabled)
{
  m_valueWidget->setEnabled(enabled);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalOption::isEnabled() const
{
  return m_valueWidget->isEnabled();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalOption::setToolTip(const QString & toolTip)
{
  m_name->setToolTip(toolTip);
  m_valueWidget->setToolTip(toolTip);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalOption::isModified() const
{
  return m_valueWidget->isModified();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalOption::getOrginalValue() const
{
  return m_valueWidget->getOriginalValue();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString GraphicalOption::getOrginalValueString() const
{
  return m_valueWidget->getOriginalString();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalOption::commit()
{
  m_valueWidget->commit();
}
