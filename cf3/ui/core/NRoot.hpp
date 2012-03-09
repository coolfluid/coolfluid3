// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_NRoot_hpp
#define cf3_ui_core_NRoot_hpp

//////////////////////////////////////////////////////////////////////////////

#include "common/UUCount.hpp"

#include "ui/core/CNode.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common { class URI; }

namespace ui {
namespace core {

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Client root.
  /// This class is wrapper for @c cf3::common::Root class on the client side.
  /// A NRoot object may never have any child. Add them to the
  /// internal @c Root componenent instead. It can be obtained by calling
  /// @c root() method.
  /// @author Quentin Gasper.

  class Core_API NRoot :
      public QObject, public CNode
  {
    Q_OBJECT

  public: // typedefs

  public:

    /// @brief Constructor
    /// @param name Node name
    NRoot(const std::string & name);

    /// @brief Gives the text to put on a tool tip
    /// @return The name of the class.
    virtual QString tool_tip() const;

    /// @brief Gets a child node bqsed on its number
    /// @param number Child number.
    /// @return Returns the child, or a null pointer if the number is not
    /// valid.
    Handle< CNode const > child_from_root(cf3::Uint number) const;

    /// @brief Gets a child node bqsed on its number
    /// @param number Child number.
    /// @return Returns the child, or a null pointer if the number is not
    /// valid.
    Handle< CNode > child_from_root(cf3::Uint number);

    /// @brief Gives the client UuiD
    /// @return Returns the Client UuiD
    const common::UUCount& uuid() const;

    /// @name SIGNALS
    // @{

    void signature_connect_server( common::SignalArgs & frame );

    void signature_disconnect_server( common::SignalArgs & frame );

    void signal_connect_server( common::SignalArgs & frame );

    void signal_disconnect_server( common::SignalArgs & frame );

    // }@

  signals:

    void connected();

  private slots:

    void network_disconnected( common::SignalArgs & args );

  protected:

    /// Disables the local signals that need to.
    /// @param localSignals Map of local signals. All values are set to true
    /// by default.
    virtual void disable_local_signals(QMap<QString, bool> & localSignals) const;

  private slots:

    /// @brief Slot called when the client is connected to the server.
    void network_connected(common::SignalArgs &);

  private :

    /// @brief Client UuiD
    common::UUCount m_uuid;
  private: // helper functions

    /// @name Signals
    //@{

    /// @brief Method called when the server sends a shutdown event.
    /// @param node Signal parameters. This parameter is not used.
    void shutdown(common::SignalArgs & args);

    /// @brief Method called when the server confirms/rejects the client
    /// registration.
    /// @param node Signal parameters. Should contain a boolean value named
    /// "accepted". If this value is @c true, the server has accepted the
    /// registration. Otherwise the server rejects the registration, in this
    /// case, the method closes the network connection.
    void client_registration(common::SignalArgs & node);

    /// @brief Method called when the server rejects a request.
    /// @param node Signal parameters. Should contain two values:
    /// @li a string named "uuid" that contains the rejected frame UuiD
    /// @li a string named "reason" that contains the reason of the reject
    void frame_rejected(common::SignalArgs & node);

    //@} END Signals



  }; // class NRoot

//////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_NRoot_hpp
