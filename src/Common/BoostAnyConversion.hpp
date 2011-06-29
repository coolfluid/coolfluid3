// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_BoostAnyConversion_hpp
#define CF_Common_BoostAnyConversion_hpp

/////////////////////////////////////////////////////////////////////////////

#include <boost/any.hpp>

#include "Common/CommonAPI.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////

Common_API std::string any_to_str( const boost::any & value );

Common_API std::string any_type( const boost::any & value );

template<typename TYPE>
Common_API TYPE any_to_value( const boost::any & value );

/////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_BoostAnyConversion_hpp
