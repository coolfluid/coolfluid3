// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Server_CCore_h
#define cf3_ui_Server_CCore_h

////////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QThread>
#include <QList>
#include <vector>
#include <string>

#include "common/Component.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common {
namespace XML {
  class XmlDoc;
}
}

namespace ui {
namespace Server {

////////////////////////////////////////////////////////////////////////////

  class ServerNetworkComm;

  /// @brief Manages everything that is not related to the network layer
  /// or the simulation management.

  /// @author Quentin Gasper

  class CCore :
      public QObject,
      public cf3::common::Component
  {
    Q_OBJECT

  private:

    struct DirContent
    {
      std::vector<std::string> dirs;
      std::vector<std::string> files;
      std::vector<Uint> fileSizes;
      std::vector<std::string> dirDates;
      std::vector<std::string> fileDates;
    };

  public:

    typedef boost::shared_ptr<CCore> Ptr;
    typedef boost::shared_ptr<CCore const> ConstPtr;

    /// @brief Constructor

    CCore();

    /// @brief Destructor

    /// Free all allocated memory.
     ~CCore();

     /// Component::derived_type_name implementation
      std::string derived_type_name() const
      {
        return common::TypeInfo::instance().portable_types[ typeid(*this).name() ];
      }
     
     /// @param hostame Host name
     /// @param portNumber Port number
     /// @throw NetworkException
     bool listenToPort(quint16 portNumber);

     void sendSignal(const common::XML::XmlDoc & signal);

     static std::string type_name() { return "CCore"; }

     void sendFrameRejected(const std::string & clientid,
                            const std::string & frameid,
                            const cf3::common::URI & sender,
                            const QString & reason);

     void sendException(const char * what,
                        const std::string & clientid = std::string());

     void forward_signal( common::SignalArgs & args );

     void sendACK( const std::string & clientid,
                   const std::string & frameid,
                   bool success,
                   const std::string & message);

  private slots:

    /// @brief Slot called when a new client connects

    /// Sends server status (file open, simulation running) to the new client.
    /// @param clientId New client id.
     void newClient(const std::string & uuid);

    /// @brief Forwards a message from the simulator to the network layer
    /// @param message Message to send
    void message(const QString & message);

    /// @brief Forwards an error message from the simulator to the network layer
    /// @param message Error message to send
    void error(const QString & message);

  public slots:

    void newEvent(const std::string & name, const cf3::common::URI & path);

  private:
    /// @brief The default path for the file browsing.

    /// The default path is the current directory (./).
    const QString DEFAULT_PATH;

    /// @brief The network communication
    ServerNetworkComm * m_commServer;

    /// @brief Indicates wether a file is already open.

    /// If @c true, a file is already open.
    bool m_fileOpen;

    /// @brief Indicates wether the simulation is running.

    /// If @c true, the simulation is running.
    bool m_simRunning;

    bool m_active;

    /// @brief Reads a directory contents.

    /// @param directory Directory to read.
    /// @param extensions List of wanted extensions. According to the network
    /// m_protocol, this list may be empty.
    /// @param includeFiles If @c true, sub-directories and files returned.
    /// If @c false, only sub-directories will be returned.
    /// @param includeNoExtension If @c true, files without any extension will
    /// be returned. If @c false, they will not.
    /// @param dirsList Reference of a @c QStringList where sub-directories
    /// names will be stored.
    /// @param filesList Reference of a @c QStringList where files names will
    /// be stored.
    /// @return Returns @c true if the directory has been correctly read.
    /// Otherwise, returns @c false (@c dirsList and @c filesList are not
    /// modified in this case).
    bool getDirContent(const QString & directory,
                       const std::vector<std::string> & extensions,
                       bool includeFiles,
                       bool includeNoExtension,
                       DirContent & content) const;

    void read_dir(cf3::common::SignalArgs & node);

    void createDir(cf3::common::SignalArgs & node);

    void shutdown(cf3::common::SignalArgs & node);

    void saveConfig(cf3::common::SignalArgs & node);

    void signal_list_tree(cf3::common::SignalArgs & node);
  };

////////////////////////////////////////////////////////////////////////////

} // Server
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Server_CCore_h
