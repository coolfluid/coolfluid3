// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_network_error_handler_hpp
#define cf3_ui_network_error_handler_hpp

#include "ui/network/LibNetwork.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace network {

//////////////////////////////////////////////////////////////////////////////

class Network_API ErrorHandler
{
public:

  virtual void error( const std::string & message )  {}

}; // ErrorHandler

//////////////////////////////////////////////////////////////////////////////

} // network
} // ui
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_network_error_handler_hpp
