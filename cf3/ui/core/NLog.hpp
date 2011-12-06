// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_NLog_hpp
#define cf3_ui_core_NLog_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QHash>

#include "common/LogStringForwarder.hpp"

#include "ui/core/CNode.hpp"

#include "ui/uicommon/LogMessage.hpp"

class QString;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace ui {
namespace core {

/////////////////////////////////////////////////////////////////////////////

  /// @brief Log component
  /// @author Quentin Gasper.

  class Core_API NLog :
      public QObject,
      public CNode,
      public common::LogStringForwarder
  {
    Q_OBJECT

  public:

    /// @brief Constructor.
    NLog();

    /// @brief Destructor.

    /// Frees all allocated memory.
    ~NLog();

    /// @brief Adds a message to the log.

    /// If the message contains '<' or '>' characters, they will replaced
    /// respectively by '&lt;' and '&gt;'.
    /// @param message The message to add.
    void add_message(const QString & message);

    /// @brief Adds an error message to the log.

    /// If the message contains '<' or '>' characters, they will replaced
    /// respectively by '&lt;' and '&gt;'.
    /// @param message The error message to add.
    void add_error(const QString & message);

    /// @brief Adds a warning message to the log.

    /// If the message contains '<' or '>' characters, they will replaced
    /// respectively by '&lt;' and '&gt;'.
    /// @param message The warning message to add.
    void add_warning(const QString & message);

    /// @brief Adds an exception message to the log.

    /// @param message The exception message to add.
    void add_exception(const QString & message);

    /// @brief Gives the text to put on a tool tip
    /// @return The name of the class.
    virtual QString tool_tip() const;

    static Handle<NLog> global();

  signals:

    /// @brief Signal emitted when a new message arrives.
    /// @param message Message text
    /// @param isError If @c true it is an error message; otherwise it is
    /// a "normal" message.
    void new_message(const QString & message, uiCommon::LogMessage::Type type);

    /// @brief Signal emitted when an exception arrives
    /// @param message Exception message
    void new_exception(const QString & message);

  protected:

    /// Disables the local signals that need to.
    /// @param localSignals Map of local signals. All values are set to true
    /// by default.
    virtual void disable_local_signals(QMap<QString, bool> & localSignals) const {}

    virtual void message ( const std::string & data );

  private:

    /// @brief Hash map that associates a type message to its name in
    /// string format.

    /// The key is the type. The value is the name.
    QHash<uiCommon::LogMessage::Type, QString> m_typeNames;

    /// @brief Boost slot called when a message comes from the server
    /// @param node Signal node
    void signal_message(common::SignalArgs & node);

    /// @brief Appends a message to the log

    /// If the message contains '<' or '>' characters, they will replaced
    /// respectively by '&lt;' and '&gt;'.
    /// @param type Message type
    /// @param fromServer If @c true, the message comes from the server;
    /// otherwise it comes from the client.
    /// @param message Message
    void append_to_log( uiCommon::LogMessage::Type type,
                        bool fromServer,
                        const QString & message );

  }; // class NLog

  ///////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

/////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_NLog_hpp
