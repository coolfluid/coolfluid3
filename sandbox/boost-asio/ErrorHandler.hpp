// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_sandbox_boost_asio_error_handler_hpp
#define cf3_sandbox_boost_asio_error_handler_hpp

#include <string>

class ErrorHandler
{
public:

  void error( const std::string & message ) const;

};

#endif // cf3_sandbox_boost_asio_error_handler_hpp
