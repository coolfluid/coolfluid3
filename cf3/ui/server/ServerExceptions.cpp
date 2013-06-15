// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "ui/server/ServerExceptions.hpp"

using namespace cf3::common;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace server {

//////////////////////////////////////////////////////////////////////////////

UnknownClientId::UnknownClientId(const CodeLocation& where,
                                                   const std::string& what)
: common::Exception(where, what, "UnknownClientId")
{}

UnknownClientId::~UnknownClientId() throw ()
{}

//////////////////////////////////////////////////////////////////////////////

NetworkError::NetworkError(const CodeLocation& where, const std::string& what)
: common::Exception(where, what, "NetworkException")
{}

NetworkError::~NetworkError() throw ()
{}

//////////////////////////////////////////////////////////////////////////////

} // Server
} // ui
} // cf3
