// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Core_NLog_hpp
#define CF_GUI_Core_NLog_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QHash>

#include "Common/LogStringForwarder.hpp"

#include "UI/Core/CNode.hpp"

#include "UI/UICommon/LogMessage.hpp"

class QString;

//////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace UI {
namespace Core {

/////////////////////////////////////////////////////////////////////////////

  /// @brief Log component
  /// @author Quentin Gasper.

  class Core_API NLog :
      public QObject,
      public CNode,
      public Common::LogStringForwarder
  {
    Q_OBJECT

  public:

    typedef boost::shared_ptr<NLog> Ptr;
    typedef boost::shared_ptr<NLog const> ConstPtr;

    /// @brief Constructor.
    NLog();

    /// @brief Destructor.

    /// Frees all allocated memory.
    ~NLog();

    /// @brief Adds a message to the log.

    /// If the message contains '<' or '>' characters, they will replaced
    /// respectively by '&lt;' and '&gt;'.
    /// @param message The message to add.
    void addMessage(const QString & message);

    /// @brief Adds an error message to the log.

    /// If the message contains '<' or '>' characters, they will replaced
    /// respectively by '&lt;' and '&gt;'.
    /// @param message The error message to add.
    void addError(const QString & message);

    /// @brief Adds a warning message to the log.

    /// If the message contains '<' or '>' characters, they will replaced
    /// respectively by '&lt;' and '&gt;'.
    /// @param message The warning message to add.
    void addWarning(const QString & message);

    /// @brief Adds an exception message to the log.

    /// @param message The exception message to add.
    void addException(const QString & message);

    /// @brief Gives the text to put on a tool tip
    /// @return The name of the class.
    virtual QString toolTip() const;

    static Ptr globalLog();

  signals:

    /// @brief Signal emitted when a new message arrives.
    /// @param message Message text
    /// @param isError If @c true it is an error message; otherwise it is
    /// a "normal" message.
    void newMessage(const QString & message, UICommon::LogMessage::Type type);

    /// @brief Signal emitted when an exception arrives
    /// @param message Exception message
    void newException(const QString & message);

  protected:

    /// Disables the local signals that need to.
    /// @param localSignals Map of local signals. All values are set to true
    /// by default.
    virtual void disableLocalSignals(QMap<QString, bool> & localSignals) const {}

    virtual void message ( const std::string & data );

  private:

    /// @brief Hash map that associates a type message to its name in
    /// string format.

    /// The key is the type. The value is the name.
    QHash<CF::UI::UICommon::LogMessage::Type, QString> m_typeNames;

    /// @brief Boost slot called when a message comes from the server
    /// @param node Signal node
    void signal_message(Common::SignalArgs & node);

    /// @brief Appends a message to the log

    /// If the message contains '<' or '>' characters, they will replaced
    /// respectively by '&lt;' and '&gt;'.
    /// @param type Message type
    /// @param fromServer If @c true, the message comes from the server;
    /// otherwise it comes from the client.
    /// @param message Message
    void appendToLog(CF::UI::UICommon::LogMessage::Type type, bool fromServer,
                     const QString & message);

  }; // class NLog

  ///////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Core_NLog_hpp
