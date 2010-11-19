// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CF.hpp"
#include "Common/URI.hpp"

#ifdef CF_HAVE_CXXABI_H
#include <cxxabi.h>
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

std::string demangle(const char* type)
{
  int status = 0;
  char* r = 0;

#ifdef CF_HAVE_CXXABI_H
  r = abi::__cxa_demangle(type, 0, 0, &status);
#else // dont have cxxabi.h
  status = 0;
#endif

  std::string ret_value;
  if ( (r == 0) || (status != 0) )
    ret_value = std::string(type);
  else
    ret_value = std::string(r);

  free(r);

  return ret_value;
}

////////////////////////////////////////////////////////////////////////////////

  } // Common

  ////////////////////////////////////////////////////////////////////////////////

  TypeInfo::TypeInfo()
  {
    regist<int>("integer");
    regist<CF::Uint>("unsigned");
    regist<std::string>("string");
    regist<bool>("bool");
    regist<CF::Real>("real");
    regist<Common::URI>("uri");
  }

  TypeInfo& TypeInfo::instance()
  {
    static TypeInfo tregister;
    return tregister;
  }

  std::string class_name_from_typeinfo (const std::type_info & info)
  {
    TypeInfo& ti = TypeInfo::instance();
    std::map<std::string, std::string>::const_iterator it =
        ti.portable_types.find(info.name());

    cf_assert_desc(Common::demangle(info.name()).c_str(), it != ti.portable_types.end() );

    return it->second;
  }

  ////////////////////////////////////////////////////////////////////////////////

} // CF
