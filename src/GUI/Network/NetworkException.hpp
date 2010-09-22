// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_network_Network_h
#define CF_network_Network_h

////////////////////////////////////////////////////////////////////////////////

#include <QString>

#include "Common/Exception.hpp"

#include "GUI/Network/LibNetwork.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Common { class CodeLocation; }

namespace GUI {
namespace Network {

////////////////////////////////////////////////////////////////////////////////

  /// @brief Exception thrown when the server can not open its socket.

  /// @author Quentin Gasper.

  class Network_API NetworkException : public CF::Common::Exception
  {
    public:

    /// Constructor
    NetworkException(const CF::Common::CodeLocation& where,
                     const std::string& what);

    /// Copy constructor
    NetworkException(const NetworkException& e) throw ();

  }; // class Network

  /////////////////////////////////////////////////////////////////////////////

} // namespace Network
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_network_Network_h
