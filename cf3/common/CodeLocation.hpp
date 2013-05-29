// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_CodeLocation_hpp
#define cf3_common_CodeLocation_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CommonAPI.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

/// This class stores the information about a location in the source code
/// @author Tiago Quintino
class Common_API CodeLocation {
public:

  /// constructor of the code location
  explicit CodeLocation (const char * file, int line, const char * function);

  /// @returns a string where the location is
  std::string str () const;

  std::string short_str() const;

private:
  /// from which file the exception was thrown
  const char * m_file;
  /// from which function the exception was thrown
  /// @note will be empty if the compiler does not support it
  const char * m_function;
  /// from which line the exception was thrown
  int          m_line;

}; // CodeLocation

////////////////////////////////////////////////////////////////////////////////

#define FromHere() cf3::common::CodeLocation( __FILE__ , __LINE__ , __FUNCTION__ )

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_CodeLocation_hpp
