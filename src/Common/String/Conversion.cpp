// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/compare.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/URI.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/String/Conversion.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace String {

////////////////////////////////////////////////////////////////////////////////

/// Operations on std::string

  /// Converts to std::string
  /// Don't use this to convert to a char, use c_str for that.
  /// Typical use is to convert to numbers.
  /// @param str string to convert from
  /// @return converter type
  
  template <>
  Common_API std::string to_str<bool> (const bool & v)
  {
    return v ? "true" : "false";
  }
  
  template <>
  Common_API std::string to_str<int> (const int & v)
  {
    return boost::lexical_cast<std::string>(v);
  }
  
  template <>
  Common_API std::string to_str<unsigned long> (const unsigned long & v)
  {
    return boost::lexical_cast<std::string>(v);
  }
  
  template <>
  Common_API std::string to_str<Uint> (const Uint & v)
  {
    return boost::lexical_cast<std::string>(v);
  }
  
  template <>
  Common_API std::string to_str<Real> (const Real & v)
  {
    std::ostringstream oss;
    oss << v;
    return oss.str();
  }

  template <>
  Common_API std::string to_str<URI> (const URI & v)
  {
    return v.string();
  }
  
  ////////////////////////////////////////////////////////////////////////////////
  
  /// Converts from std::string
  /// Don't use this to convert to a char, use c_str for that.
  /// Typical use is to convert to numbers.
  /// @param str string to convert from
  /// @return converter type
  template <>
  Common_API bool from_str<bool> (const std::string& str)
  {
    bool match = false;
    boost::algorithm::is_equal test_equal;

    if ( test_equal(str,"true") ||
         test_equal(str,"on")   ||
         test_equal(str,"1")     )
    {
      return true;
    }

    if ( test_equal(str,"false") ||
         test_equal(str,"off")   ||
         test_equal(str,"0")      )
    {
      return false;
    }

    if (!match)
      throw ParsingFailed (FromHere(), "Incorrect option conversion to bool of string [" + str + "]" );
    return true;
  }
  
  template <>
  Common_API int from_str<int> (const std::string& str)
  {
    return boost::lexical_cast<int> (str );
  }

  template <>
  Common_API unsigned long from_str<unsigned long> (const std::string& str)
  {
    return boost::lexical_cast<unsigned long> ( str );
  }
  
  template <>
  Common_API Uint from_str<Uint> (const std::string& str)
  {
    return boost::lexical_cast<Uint> ( str );
  }

  template <>
  Common_API Real from_str<Real> (const std::string& str)
  {
    return boost::lexical_cast<Real> ( str );
  }

  template <>
  Common_API URI from_str<URI> (const std::string& str)
  {
    return URI( str );
  }

////////////////////////////////////////////////////////////////////////////////

} // String
} // Common
} // CF
