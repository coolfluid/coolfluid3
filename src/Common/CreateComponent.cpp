// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>

#include "CreateComponent.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////

std::string library_name(const std::string& builder_name)
{
  // Copy holding the result
  std::string result = builder_name;
  
  // Strip the class name
  boost::erase_tail(result, result.end() - boost::find_last(result, ".").begin());
  
  if(boost::starts_with(result, "CF."))
    boost::replace_first(result, "CF", "coolfluid");
  
  boost::replace_all(result, ".", "_");
  boost::to_lower(result);
  
  return result;
}


/////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
