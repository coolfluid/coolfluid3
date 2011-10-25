// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Graphics_LoggingList_h
#define cf3_GUI_Graphics_LoggingList_h

////////////////////////////////////////////////////////////////////////////////

#include <QTextEdit>

#include "UI/UICommon/LogMessage.hpp"

#include "UI/Graphics/LibGraphics.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Manages a graphical log component.

  class Graphics_API LoggingList : public QTextEdit
  {
    Q_OBJECT

  public:

    /// @brief Constructor

    /// @param parent Parent. May be @c nullptr.
    /// @param maxLogLines Number of lines before the log must be cleared. If 0,
    /// the log is never cleared.
    LoggingList(QWidget * parent = nullptr, unsigned int max_log_lines = 100000);

    /// @brief Destructor

    /// Frees all allocated memory. Parent is not destroyed.
    ~LoggingList();

    /// @brief Sets the maximum number of lines before the log is cleared.

    /// @param maxLogLines Maximum number of lines. If 0, the log is never
    /// cleared.
    /// @warning Be careful when modifying this value. The more lines the log
    /// contains, the more it takes memory to store its content.
    void set_max_log_lines(unsigned int max_log_lines = 100000);

    /// @brief Gives the maximum number of lines before the log is cleared.

    /// @return Returns the maximum number of lines before the log is cleared.
    unsigned int max_log_lines() const;

    /// @brief Gives the number of lines the log contains.

    /// @return Returns the number of lines the log contains.
    unsigned int log_lines_count() const;

  public slots:

    /// @brief Clears the log

    /// Use this method instead of the base class method @c clear() to ensure
    /// that the line counter is reset.
    void clear_log();

  private slots:

    void new_message(const QString & message, UICommon::LogMessage::Type type);

  private:

    /// @brief Number of lines in the log.
    unsigned int m_log_lines_count;

    /// @brief Number of lines before the log must be cleared.

    /// If 0, the log is never cleared.
    unsigned int m_max_log_lines;

  }; // class LoggingList

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Graphics_LoggingList_h
