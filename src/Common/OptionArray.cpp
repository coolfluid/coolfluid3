// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/filesystem/path.hpp>

#include "Common/OptionArray.hpp"
#include "Common/URI.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  OptionArray::OptionArray(const std::string& name, const std::string& type,
                           const std::string& desc, const boost::any def) :
      Option(name,type, desc, def)
  {

  }


////////////////////////////////////////////////////////////////////////////////
  template<>
  Common_API const char * OptionArrayT<bool>::elem_type() const { return "bool"; }

  template<>
  Common_API const char * OptionArrayT<int>::elem_type() const { return "integer"; };

  template<>
  Common_API const char * OptionArrayT<CF::Uint>::elem_type() const { return "unsigned"; }

  template<>
  Common_API const char * OptionArrayT<CF::Real>::elem_type() const { return "real"; }

  template<>
  Common_API const char * OptionArrayT<std::string>::elem_type() const { return "string"; }

  /// @todo: temporary should go
  template<>
  Common_API const char * OptionArrayT<boost::filesystem::path>::elem_type() const { return "file"; }

  template<>
  Common_API const char * OptionArrayT<URI>::elem_type() const { return "uri"; }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
