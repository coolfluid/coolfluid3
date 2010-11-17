// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_ClientCore_hpp
#define CF_GUI_Client_Core_ClientCore_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "Common/XmlHelpers.hpp"

#include "GUI/Client/Core/TSshInformation.hpp"

#include "GUI/Client/Core/CNode.hpp"

#include "GUI/Client/Core/LibClientCore.hpp"

class QProcess;
class QString;
class QTimer;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {

namespace Network {
  class ComponenentType;
  struct HostInfos;
}

namespace ClientCore {

////////////////////////////////////////////////////////////////////////////////

  class ClientNetworkComm;
  class StatusModel;

////////////////////////////////////////////////////////////////////////////////

  /// @brief Core of the client application.

  /// This class is an interface between the client network layer and the rest
  /// of the application.

  /// @author Quentin Gasper.

  class ClientCore_API NCore :
      public QObject,
      public CNode
  {
    Q_OBJECT

  public:

    typedef boost::shared_ptr<const NCore> ConstPtr;
    typedef boost::shared_ptr<NCore> Ptr;

    /// @brief Constructor
    NCore();

    /// @brief Destructor
    ~NCore();

    /// @brief Sends a signal to the network layer

    /// @param signal The signal to send. Build the signal using @c #XmlOps and
    /// @c #XmlParams classes.
    void sendSignal(CF::Common::XmlDoc & signal);

    /// @brief Attempts to connect to a server.

    /// @param sshInfo Connection information
    void connectToServer(const TSshInformation & sshInfo);

    /// @brief Disconnects from the server
    /// @param shutdown If @c true, a request to shutdwn the server application
    /// is sent before the disconnection.
    void disconnectFromServer(bool shutdown);

    /// @brief Gives the text to put on a tool tip
    /// @return The name of the class.
    virtual QString toolTip() const;

  public slots:

    /// @brief Sends a request to update de tree
    void updateTree();

  private slots:

    /// @brief Slot called when the client is connected to the server.
    void connected();

  signals:

    /// @brief Signal emitted when a connection between the client and the server
    /// has been established.
    void connectedToServer();

    /// @brief Signal emitted when the connection between the client and the
    /// server has been closed.
    void disconnectedFromServer();

  private: // data

    /// @brief The network layer
    ClientNetworkComm * m_networkComm;

    /// @brief Timer used on server launchin process.

    /// On timeout, the client attemps to connect to the server.
    QTimer * m_timer;

    /// @brief The launching process.
    QProcess * m_process;

    /// @brief The current connection information.
    TSshInformation m_commSshInfo;

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

    /// @name Signals
    //@{

    /// @brief Method called when the server sends a shutdown event.
    /// @param node Signal parameters. This parameter is not used.
    void shutdown(CF::Common::XmlNode & node);

    /// @brief Method called when the server confirms/rejects the client
    /// registration.
    /// @param node Signal parameters. Should contain a boolean value named
    /// "accepted". If this value is @c true, the server has accepted the
    /// registration. Otherwise the server rejects the registration, in this
    /// case, the method closes the network connection.
    void client_registration(CF::Common::XmlNode & node);

    /// @brief Method called when the server rejects a request.
    /// @param node Signal parameters. Should contain two values:
    /// @li a string named "uuid" that contains the rejected frame UUID
    /// @li a string named "reason" that contains the reason of the reject
    void frame_rejected(CF::Common::XmlNode & node);

    //@} END Signals

  }; // class ClientCore

////////////////////////////////////////////////////////////////////////////////

} // namespace ClientCore
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_ClientCore_hpp
