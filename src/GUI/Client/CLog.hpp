#ifndef CF_GUI_Client_CLog_hpp
#define CF_GUI_Client_CLog_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QHash>

#include "Common/Component.hpp"

#include "GUI/Network/LogMessage.hpp"

class QString;

//////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace GUI {
namespace Client {

/////////////////////////////////////////////////////////////////////////////

  class CLog :
      public QObject,
      public CF::Common::Component
  {
    Q_OBJECT

  public:

    typedef boost::shared_ptr<CLog> Ptr;
    typedef boost::shared_ptr<CLog const> ConstPtr;

    CLog();

    ~CLog();

    /// @brief Adds a message to the log.

    /// If the message is
    /// @param message The message to add.
    void addMessage(const QString & message);

    void addError(const QString & message);

    void addException(const QString & message);

  signals:

    void newMessage(const QString & message, bool isError);

    void newException(const QString & message);

  private:

    QHash<CF::GUI::Network::LogMessage::Type, QString> m_typeNames;

    CF::Common::Signal::return_t message(CF::Common::Signal::arg_t & node);

    CF::Common::Signal::return_t list_tree(CF::Common::Signal::arg_t & node);

    void appendToLog(CF::GUI::Network::LogMessage::Type type, bool fromServer,
                     const QString & message);

  }; // class CLog

  ///////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_CLog_hpp
