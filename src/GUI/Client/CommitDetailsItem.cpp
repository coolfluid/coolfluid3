#include <QtCore>

#include "GUI/Client/CommitDetailsItem.hpp"

using namespace CF::GUI::Client;

CommitDetailsItem::CommitDetailsItem(const QString & optionName,
                                     const QString & oldValue,
                                     const QString & currentValue)
{
  m_optionName = optionName;
  m_oldValue = oldValue;
  m_currentValue = currentValue;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString CommitDetailsItem::getCurrentValue() const
{
  return m_currentValue;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString CommitDetailsItem::getOldValue() const
{
  return m_oldValue;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString CommitDetailsItem::getOptionName() const
{
  return m_optionName;
}
