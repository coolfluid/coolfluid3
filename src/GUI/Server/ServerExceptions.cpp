// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "GUI/Server/ServerExceptions.hpp"

using namespace CF::Common;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Server {

//////////////////////////////////////////////////////////////////////////////

UnknownClientId::UnknownClientId(const CodeLocation& where,
                                                   const std::string& what)
: Common::Exception(where, what, "UnknownClientId")
{}

UnknownClientId::~UnknownClientId() throw ()
{}

//////////////////////////////////////////////////////////////////////////////

NetworkError::NetworkError(const CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "NetworkException")
{}

NetworkError::~NetworkError() throw ()
{}

//////////////////////////////////////////////////////////////////////////////

} // Server
} // GUI
} // CF
