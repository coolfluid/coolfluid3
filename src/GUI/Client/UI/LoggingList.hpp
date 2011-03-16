// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_LoggingList_h
#define CF_GUI_Client_UI_LoggingList_h

////////////////////////////////////////////////////////////////////////////////

#include <QTextEdit>

#include "GUI/Network/LogMessage.hpp"

#include "GUI/Client/UI/LibClientUI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Manages a graphical log component.

  class ClientUI_API LoggingList : public QTextEdit
  {
    Q_OBJECT

  public:

    /// @brief Constructor

    /// @param parent Parent. May be @c nullptr.
    /// @param maxLogLines Number of lines before the log must be cleared. If 0,
    /// the log is never cleared.
    LoggingList(QWidget * parent = nullptr, unsigned int maxLogLines = 100000);

    /// @brief Destructor

    /// Frees all allocated memory. Parent is not destroyed.
    ~LoggingList();

    /// @brief Sets the maximum number of lines before the log is cleared.

    /// @param maxLogLines Maximum number of lines. If 0, the log is never
    /// cleared.
    /// @warning Be careful when modifying this value. The more lines the log
    /// contains, the more it takes memory to store its content.
    void setMaxLogLines(unsigned int maxLogLines = 100000);

    /// @brief Gives the maximum number of lines before the log is cleared.

    /// @return Returns the maximum number of lines before the log is cleared.
    unsigned int maxLogLines() const;

    /// @brief Gives the number of lines the log contains.

    /// @return Returns the number of lines the log contains.
    unsigned int logLinesCount() const;

  public slots:

    /// @brief Clears the log

    /// Use this method instead of the base class method @c clear() to ensure
    /// that the line counter is reset.
    void clearLog();

  private slots:

    void newMessage(const QString & message, CF::GUI::Network::LogMessage::Type type);

  private:

    /// @brief Number of lines in the log.
    unsigned int m_logLinesCounter;

    /// @brief Number of lines before the log must be cleared.

    /// If 0, the log is never cleared.
    unsigned int m_maxLogLines;

  }; // class LoggingList

////////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_LoggingList_h
