// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QString>

#include "GUI/Client/Core/CommitDetailsItem.hpp"

using namespace CF::GUI::ClientCore;

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

QString CommitDetailsItem::currentValue() const
{
  return m_currentValue;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString CommitDetailsItem::oldValue() const
{
  return m_oldValue;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString CommitDetailsItem::optionName() const
{
  return m_optionName;
}
