// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Server_ServerExceptions_hpp
#define CF_GUI_Server_ServerExceptions_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Exception.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Server {

//////////////////////////////////////////////////////////////////////////////

/// @brief Exception thrown when a given client id could not be associated to
/// any existing client.

/// @author Quentin Gasper.
class UnknownClientId : public CF::Common::Exception
{
public:

    /// Constructor
    UnknownClientId(const CF::Common::CodeLocation& where,
                             const std::string& what);

    virtual ~UnknownClientId() throw ();

}; // class UnknownClientId

//////////////////////////////////////////////////////////////////////////////

/// @brief Exception thrown when the server can not open its socket.

/// @author Quentin Gasper.

class NetworkError : public CF::Common::Exception
{
  public:

  /// Constructor
  NetworkError(const CF::Common::CodeLocation& where, const std::string& what);

  /// Copy constructor
  virtual ~NetworkError() throw ();

}; // class NetworkError

//////////////////////////////////////////////////////////////////////////////

} // Server
} // UI
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Server_ServerExceptions_hpp
