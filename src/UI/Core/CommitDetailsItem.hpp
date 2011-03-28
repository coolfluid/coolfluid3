// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Core_CommiDetailsItem_h
#define CF_GUI_Core_CommiDetailsItem_h

////////////////////////////////////////////////////////////////////////////////

#include "UI/Core/LibCore.hpp"

class QString;

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

  ////////////////////////////////////////////////////////////////////////////////

  /// @brief Basic item used by CommitDetails class.
  /// @author Quentin Gasper.

  class Core_API CommitDetailsItem
  {
  public:

    /// @brief Constructor

    /// @param optionName Option name.
    /// @param oldValue Old value. May be empty.
    /// @param currentValue Current value. May be empty.
    CommitDetailsItem(const QString & optionName, const QString & oldValue,
                      const QString & currentValue);

    /// @brief Gives the current value.
    /// @return Returns the current value.
    QString currentValue() const;

    /// @brief Give the old value.
    /// @return Returns the old value
    QString oldValue() const;

    /// @brief Gives the option name
    /// @return Returns the option name
    QString optionName() const;

  private:

    /// @brief Option name
    QString m_optionName;

    /// @brief Old value
    QString m_oldValue;

    /// @brief Current value
    QString m_currentValue;


  }; // class CommitDetailsItem

  ////////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Core_CommiDetailsItem_h
