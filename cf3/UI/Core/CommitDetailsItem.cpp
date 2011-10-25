// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QString>

#include "UI/Core/CommitDetailsItem.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

CommitDetailsItem::CommitDetailsItem(const QString & option_name,
                                     const QString & old_value,
                                     const QString & current_value)
{
  m_option_name = option_name;
  m_old_value = old_value;
  m_current_value = current_value;
}

////////////////////////////////////////////////////////////////////////////

QString CommitDetailsItem::current_value() const
{
  return m_current_value;
}

////////////////////////////////////////////////////////////////////////////

QString CommitDetailsItem::old_value() const
{
  return m_old_value;
}

////////////////////////////////////////////////////////////////////////////

QString CommitDetailsItem::option_name() const
{
  return m_option_name;
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3
