// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_XML_MultiArray_hpp
#define CF_Common_XML_MultiArray_hpp

////////////////////////////////////////////////////////////////////////////

#include "Common/BoostArray.hpp"

#include "Common/XML/Map.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace XML {

////////////////////////////////////////////////////////////////////////////

/// Adds a multi array in the provided @c Map
XmlNode add_multi_array_in(Map & map, const std::string & name,
                           const boost::multi_array<Real, 2> & array,
                           const std::string & delimiter = ";",
                           const std::vector<std::string> & labels = std::vector<std::string>());

void get_multi_array(const Map & map, const std::string & name,
                         boost::multi_array<Real, 2> & array,
                         std::vector<std::string> & labels);

////////////////////////////////////////////////////////////////////////////

} // XML
} // Common
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // MULTIARRAY_HPP
