// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Server_CCore_h
#define cf3_ui_Server_CCore_h

////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>

#include <QDir>
#include <QStringList>
#include <QThread>

#include "common/Component.hpp"

////////////////////////////////////////////////////////////////////////////

class QSettings;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common {
namespace XML {
  class XmlDoc;
}
}

namespace ui {
namespace server {

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
      std::vector<Uint> file_sizes;
      std::vector<std::string> dir_dates;
      std::vector<std::string> file_dates;
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
     bool listen_to_port(quint16 portNumber);

     void send_signal(const common::XML::XmlDoc & signal);

     static std::string type_name() { return "CCore"; }

     void forward_signal( common::SignalArgs & args );

     void send_ack( const std::string & clientid,
                    const std::string & frameid,
                    bool success,
                    const std::string & message );

  private slots:

    /// @brief Slot called when a new client connects

    /// Sends server status (file open, simulation running) to the new client.
    /// @param clientId New client id.
     void new_client(const std::string & uuid);

  private: // data
    /// @brief The default path for the file browsing.

    /// The default path is the current directory (./).
    const QString DEFAULT_PATH;

    /// @brief The network communication
    ServerNetworkComm * m_comm_server;

    QSettings * m_settings;

    /// List of user's favorite directories.
    QStringList m_favorite_directories;

  private: // functions

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
    bool get_dir_content( const QString & directory,
                          const std::vector<std::string> & extensions,
                          bool includeFiles,
                          bool includeNoExtension,
                          DirContent & content ) const;

    void read_dir(cf3::common::SignalArgs & node);

    void read_special_dir(cf3::common::SignalArgs & node);

    void shutdown(cf3::common::SignalArgs & node);

    void signal_set_favorites(cf3::common::SignalArgs & node);

    void signal_list_favorites(cf3::common::SignalArgs & node);
  };

////////////////////////////////////////////////////////////////////////////

} // Server
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Server_CCore_h
