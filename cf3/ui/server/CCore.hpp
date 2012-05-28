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
      public common::Component
  {

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
     bool listen_to_port(cf3::Uint portNumber);

     void send_signal( common::XML::SignalFrame & signal );

     static std::string type_name() { return "CCore"; }

     void forward_signal( common::SignalArgs & args );

     void send_ack( const std::string & clientid,
                    const std::string & frameid,
                    bool success,
                    const std::string & message );

  private: // functions

    /// @brief Slot called when a new client connects

    /// Sends server status (file open, simulation running) to the new client.
    /// @param args Signal arguments.
     void new_client( common::SignalArgs & args);

  private: // data
    /// @brief The default path for the file browsing.

    /// The default path is the current directory (./).
    std::string default_path;

    /// @brief The network communication
    ServerNetworkComm * m_comm_server;

//    QSettings * m_settings;

    /// List of user's favorite directories.
//    QStringList m_favorite_directories;

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
    bool get_dir_content( const std::string & directory,
                          const std::vector<std::string> & extensions,
                          bool includeFiles,
                          bool includeNoExtension,
                          DirContent & content ) const;

    void read_dir( common::SignalArgs & node );

    void read_special_dir( common::SignalArgs & node );

    void shutdown( common::SignalArgs & node );

    void signal_set_favorites( common::SignalArgs & node );

    void signal_list_favorites( common::SignalArgs & node );

    /// @brief ask the server to do an scp command to copy files from the server or from the client
    /// @param commands std::vector<std::string> containing the parameters of the scp command
    /// each time a string with length 0 appear it means that it is a new scp command that should be done
    void signal_copy_request( common::SignalArgs & node );
  };

////////////////////////////////////////////////////////////////////////////

} // Server
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Server_CCore_h
