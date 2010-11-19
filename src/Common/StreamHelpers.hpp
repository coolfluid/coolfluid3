// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_StreamHelpers_hpp
#define CF_Common_StreamHelpers_hpp

#include <Common/CF.hpp>

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
  
////////////////////////////////////////////////////////////////////////////////

// Some common functions to help outputtung data to streams

/// Print a vector enclosed in prefix and suffix, separated with the given separator
template<typename VectorT, typename StreamT>
void print_vector(StreamT& stream, const VectorT& vector, const std::string& sep=" ", const std::string& prefix = "", const std::string& suffix = "")
{
  stream << prefix;
  const Uint vector_size = vector.size();
  for(Uint i = 0; i != vector_size; ++i)
  {
    stream << (i != 0 ? sep : "") << vector[i];
  }
  stream << suffix;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_StreamHelpers_hpp
