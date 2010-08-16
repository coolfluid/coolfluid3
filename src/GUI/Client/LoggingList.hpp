#ifndef CF_GUI_Client_LoggingList_h
#define CF_GUI_Client_LoggingList_h

////////////////////////////////////////////////////////////////////////////////

#include <QHash>
#include <QTextEdit>

#include "Common/CF.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Manages a graphical log component.

  class LoggingList : public QTextEdit
  {
    Q_OBJECT

  public:

    /// @brief Constructor

    /// @param parent Parent. May be @c CFNULL.
    /// @param maxLogLines Number of lines before the log must be cleared. If 0,
    /// the log is never cleared.
    LoggingList(QWidget * parent = CFNULL, unsigned int maxLogLines = 100000);

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
    unsigned int getMaxLogLines() const;

    /// @brief Gives the number of lines the log contains.

    /// @return Returns the number of lines the log contains.
    unsigned int getLogLinesCounter() const;

  public slots:

    /// @brief Clears the log

    /// Use this method instead of the base class method @c clear() to ensure
    /// that the line counter is reset.
    void clearLog();

  private slots:

      void newMessage(const QString & message, bool isError);

  private:

    /// @brief Number of lines in the log.
    unsigned int m_logLinesCounter;

    /// @brief Number of lines before the log must be cleared.

    /// If 0, the log is never cleared.
    unsigned int m_maxLogLines;

  }; // class LoggingList

////////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_LoggingList_h
