// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_NetworkInfo_hpp
#define cf3_common_NetworkInfo_hpp

#include "common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

  /// Holds network information of the GUI.
  /// This class allows to access these pieces of information from inside the
  /// CF kernel or plugins without calling Qt libraries. Values are typically
  /// set by the GUI.
  /// @author Quentin Gasper.
  class Common_API NetworkInfo
  {

  public: // methods

    /// Constructor.
    NetworkInfo();

    /// Destructor.
    ~NetworkInfo();

    /// @return Returns @c true if this CF instance is managed by the server
    bool is_server () const;

    /// @return Returns @c true if this CF instance is managed by the client
    bool is_client () const;

    /// @return Returns the current hostname.
    std::string hostname () const;

    /// @return Returns the current port.
    unsigned short port () const;

    /// "Starts" the server.
    /// If the client is already "started", it is "stopped".
    /// @post @c is_server() returns @c true
    void start_server ();

    /// "Stops" the server.
    /// @post @c is_server() returns @c false
    void stop_server ();

    /// "Starts" the client.
    /// If the server is already "started", it is "stopped".
    /// @post @c is_client() returns @c true
    void start_client ();

    /// "Stops" the client.
    /// @post @c is_client() returns @c false
    void stop_client ();

    /// Sets the hostname.
    /// @param name The new hostname. Should be different from "localhost".
    void set_hostname ( const std::string & name );

    /// Sets the port.
    /// @param port The new port number.
    void set_port ( unsigned short port );

  private: // data

    /// Indicates whether a server manages this CF instance.
    bool m_is_server;

    /// Indicates whether a client manages this CF instance.
    bool m_is_client;

    /// The current hostname.
    std::string m_hostname;

    /// The current port number.
    unsigned short m_port;

  }; // NetworkInfo

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_NetworkInfo_hpp
