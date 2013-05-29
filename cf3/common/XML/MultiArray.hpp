// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_XML_MultiArray_hpp
#define cf3_common_XML_MultiArray_hpp

////////////////////////////////////////////////////////////////////////////

#include "common/BoostArray.hpp"

#include "common/XML/Map.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
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
} // common
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // MULTIARRAY_HPP
