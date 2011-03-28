// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Core_NRoot_hpp
#define CF_GUI_Core_NRoot_hpp

//////////////////////////////////////////////////////////////////////////////

#include <boost/uuid/uuid.hpp>

#include "Common/CRoot.hpp"

#include "GUI/Core/CNode.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Common { class URI; }

namespace GUI {
namespace Core {

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Client root.
  /// This class is wrapper for @c CF::Common::CRoot class on the client side.
  /// A NRoot object may never have any child. Add them to the
  /// internal @c CRoot componenent instead. It can be obtained by calling
  /// @c root() method.
  /// @author Quentin Gasper.

  class Core_API NRoot :
      public QObject, public CNode
  {
    Q_OBJECT

  public: // typedefs

  public:

    typedef boost::shared_ptr<NRoot> Ptr;
    typedef boost::shared_ptr<NRoot const> ConstPtr;

    /// @brief Constructor
    /// @param name Node name
    NRoot(const QString & name);

    /// @brief Gives the text to put on a tool tip
    /// @return The name of the class.
    virtual QString toolTip() const;

    /// @brief Gives the CRoot internal shared pointer
    /// @return Returns the CRoot internal shared pointer
    inline CF::Common::CRoot::Ptr root() const
    {
      return m_root;
    }

    /// @brief Gets a child node from the internal CRoot component
    /// @param number Child number.
    /// @return Returns the child, or a null pointer if the number is not
    /// valid.
    CNode::Ptr childFromRoot(CF::Uint number) const;

    /// @brief Gives the client UUID
    /// @return Returns the Client UUID
    std::string uuid() const;

    /// @name SIGNALS
    // @{

    void signature_connect_server( Common::SignalArgs & frame );

    void signature_disconnect_server( Common::SignalArgs & frame );

    void signal_connect_server( Common::SignalArgs & frame );

    void signal_disconnect_server( Common::SignalArgs & frame );

    // }@

  signals:

    void connected();

  private slots:

    void disconnected();

  protected:

    /// Disables the local signals that need to.
    /// @param localSignals Map of local signals. All values are set to true
    /// by default.
    virtual void disableLocalSignals(QMap<QString, bool> & localSignals) const;

  private slots:

    /// @brief Slot called when the client is connected to the server.
    void connectedToServer();

  private :

    /// @brief The internal CRoot component
    CF::Common::CRoot::Ptr m_root;

    /// @brief Client UUID
    boost::uuids::uuid m_uuid;
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



  }; // class NRoot

//////////////////////////////////////////////////////////////////////////////

} // Core
} // GUI
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Core_NRoot_hpp
