// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Core_CommiDetailsItem_h
#define cf3_GUI_Core_CommiDetailsItem_h

////////////////////////////////////////////////////////////////////////////////

#include "UI/Core/LibCore.hpp"

class QString;

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Core {

  ////////////////////////////////////////////////////////////////////////////////

  /// @brief Basic item used by CommitDetails class.
  /// @author Quentin Gasper.

  class Core_API CommitDetailsItem
  {
  public:

    /// @brief Constructor

    /// @param option_name Option name.
    /// @param old_value Old value. May be empty.
    /// @param current_value Current value. May be empty.
    CommitDetailsItem( const QString & option_name,
                       const QString & old_value,
                       const QString & current_value );

    /// @brief Gives the current value.
    /// @return Returns the current value.
    QString current_value() const;

    /// @brief Give the old value.
    /// @return Returns the old value
    QString old_value() const;

    /// @brief Gives the option name
    /// @return Returns the option name
    QString option_name() const;

  private:

    /// @brief Option name
    QString m_option_name;

    /// @brief Old value
    QString m_old_value;

    /// @brief Current value
    QString m_current_value;


  }; // class CommitDetailsItem

  ////////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Core_CommiDetailsItem_h
