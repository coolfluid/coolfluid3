// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cstdlib>

#include "common/TypeInfo.hpp"
#include "common/URI.hpp"
#include "common/UUCount.hpp"

#ifdef CF3_HAVE_CXXABI_H
#include <cxxabi.h>
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

std::string demangle(const char* type)
{
  int status = 0;
  char* r = 0;

#ifdef CF3_HAVE_CXXABI_H
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

TypeInfo::TypeInfo()
{
  regist<int>("integer");
  regist<cf3::Uint>("unsigned");
  regist<unsigned long>("unsigned_long");
  regist<unsigned long long>("unsigned_long_long");
  regist<std::string>("string");
  regist<bool>("bool");
  regist<cf3::Real>("real");
  regist<common::URI>("uri");
  regist<common::UUCount>("uucount");
  regist<std::vector<int> >("array[integer]");
  regist<std::vector<Uint> >("array[unsigned]");
  regist<std::vector<std::string> >("array[string]");
  regist<std::vector<bool> >("array[bool]");
  regist<std::vector<Real> >("array[real]");
  regist<std::vector<common::URI> >("array[uri]");
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

  cf3_assert_desc(common::demangle(info.name()).c_str(), it != ti.portable_types.end() );

  return it->second;
}

} // common
} // cf3
