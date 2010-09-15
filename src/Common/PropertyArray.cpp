// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/filesystem/path.hpp>

#include "Common/PropertyArray.hpp"
#include "Common/URI.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  PropertyArray::PropertyArray(const std::string& name, const std::string& type,
                           const std::string& desc, const boost::any def,
                           bool is_option) :
      Property(name,type, desc, def, is_option)
  {

  }


////////////////////////////////////////////////////////////////////////////////
  template<>
  Common_API const char * PropertyArrayT<bool>::elem_type() const { return "bool"; }

  template<>
  Common_API const char * PropertyArrayT<int>::elem_type() const { return "integer"; };

  template<>
  Common_API const char * PropertyArrayT<CF::Uint>::elem_type() const { return "unsigned"; }

  template<>
  Common_API const char * PropertyArrayT<CF::Real>::elem_type() const { return "real"; }

  template<>
  Common_API const char * PropertyArrayT<std::string>::elem_type() const { return "string"; }

  /// @todo: temporary should go
  template<>
  Common_API const char * PropertyArrayT<boost::filesystem::path>::elem_type() const { return "file"; }

  template<>
  Common_API const char * PropertyArrayT<URI>::elem_type() const { return "uri"; }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
