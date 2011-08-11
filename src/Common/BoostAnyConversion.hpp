// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

/// @file Helper functions for boost::any manipulations
/// @author Quentin Gasper.

/// Converts a boost::any value to a string.

/// The volue must of one of the types supported by the Property facility:
/// bool, int, Real, Uint, std::string or URI.
/// @param value The value to convert
/// @return Returns a string repensation of the value
/// @throw ProtocolError if the value type is not supported
/// @throw CastingFailed if the conversion failed.
Common_API std::string any_to_str( const boost::any & value );

/// Gives the demangled type string of a boost::any value.

/// @param value The value
/// @return Returns the demangled type string. May return an empty string if
/// the type is not registered to CF::Common::TypeInfo
Common_API std::string any_type( const boost::any & value );

/// Converts a boost::any value to the provided TYPE.

/// @param value The value to convert.
/// @return Returns the converted value.
/// @throw CastingFailed if the value could not be converted.
template<typename TYPE>
Common_API TYPE any_to_value( const boost::any & value );

/////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_BoostAnyConversion_hpp
