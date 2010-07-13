#ifndef CF_GUI_Client_NLog_hpp
#define CF_GUI_Client_NLog_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QHash>
#include <QList>

#include "GUI/Client/CNode.hpp"

#include "GUI/Network/LogMessage.hpp"

class QString;
class QIcon;

//////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace GUI {
namespace Client {

/////////////////////////////////////////////////////////////////////////////

  class NLog :
      public QObject,
      public CNode
  {
    Q_OBJECT

  public:

    typedef boost::shared_ptr<NLog> Ptr;
    typedef boost::shared_ptr<NLog const> ConstPtr;

    NLog();

    ~NLog();

    /// @brief Adds a message to the log.

    /// If the message is
    /// @param message The message to add.
    void addMessage(const QString & message);

    void addError(const QString & message);

    void addException(const QString & message);

    /// @brief Gives the icon associated to this node
    /// @return Returns the icon associated to this node
    /// @note This method should be reimplemented by all subclasses.
    virtual QIcon getIcon() const;

    virtual QString getToolTip() const;

    virtual void getOptions(QList<NodeOption> & params) const;

    virtual bool isClientComponent() const { return true; }

  signals:

    void newMessage(const QString & message, bool isError);

    void newException(const QString & message);

  private:

    QHash<CF::GUI::Network::LogMessage::Type, QString> m_typeNames;

    CF::Common::Signal::return_t message(CF::Common::Signal::arg_t & node);

    CF::Common::Signal::return_t list_tree(CF::Common::Signal::arg_t & node);

    void appendToLog(CF::GUI::Network::LogMessage::Type type, bool fromServer,
                     const QString & message);

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}


  }; // class NLog

  ///////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_NLog_hpp
