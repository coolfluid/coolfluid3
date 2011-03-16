// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_ClientCore_hpp
#define CF_GUI_Client_Core_ClientCore_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "GUI/Client/Core/TSshInformation.hpp"
#include "GUI/Client/Core/CNode.hpp"

class QString;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {

namespace Network {
  class ComponenentType;
  struct HostInfos;
}

namespace ClientCore {

////////////////////////////////////////////////////////////////////////////////

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

    virtual QString toolTip() const;

    static Ptr globalCore();

  private slots:

    /// @brief Slot called when the client is connected to the server.
    void connectedToServer();

    /// @brief Slot called when the client is disconnected from the server.
//    void disconnected();

  private: // data

    /// @brief The current connection information.
    TSshInformation m_commSshInfo;

  private: // helper functions
    /// @name Signals
    //@{

    /// @brief Method called when the server sends a shutdown event.
    /// @param node Signal parameters. This parameter is not used.
    void shutdown(Common::SignalArgs & node);

    /// @brief Method called when the server confirms/rejects the client
    /// registration.
    /// @param node Signal parameters. Should contain a boolean value named
    /// "accepted". If this value is @c true, the server has accepted the
    /// registration. Otherwise the server rejects the registration, in this
    /// case, the method closes the network connection.
    void client_registration(Common::SignalArgs & node);

    /// @brief Method called when the server rejects a request.
    /// @param node Signal parameters. Should contain two values:
    /// @li a string named "uuid" that contains the rejected frame UUID
    /// @li a string named "reason" that contains the reason of the reject
    void frame_rejected(Common::SignalArgs & node);

    //@} END Signals

  }; // class ClientCore

////////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_ClientCore_hpp
