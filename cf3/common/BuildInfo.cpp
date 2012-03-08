// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "coolfluid-config.hpp"

#include <sstream>

#include "common/BuildInfo.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::build_type () const
{
  return CF3_BUILD_TYPE;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::git_commit_sha () const
{
  return CF3_GIT_COMMIT_SHA;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::git_commit_date () const
{
  return CF3_GIT_COMMIT_DATE;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::release_version () const
{
  return CF3_VERSION_STR;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::kernel_version () const
{
  return CF3_KERNEL_VERSION_STR;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::build_processor () const
{
  return CF3_BUILD_PROCESSOR;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::build_tool () const
{
  std::string ret;
#ifdef CF3_CMAKE_VERSION
  ret += "CMake ";
  ret += CF3_CMAKE_VERSION;
#else
  ret += "UNKNOWN";
#endif
  return ret;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::os_name() const
{
  return CF3_OS_NAME;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::os_long_name() const
{
  return CF3_OS_LONGNAME;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::os_version() const
{
  return CF3_OS_VERSION;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::os_bits() const
{
  return CF3_OS_BITS;
}
////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::version_header() const
{
  std::ostringstream out;

  out << "Release      : " << release_version() << "\n";
  out << "Kernel       : " << kernel_version()  << "\n";
  out << "Build System : " << build_tool()    << "\n";
  out << "Build Type   : " << build_type()      << "\n";
  out << "Build OS     : " << os_long_name() << " [" << os_bits() << "bits]\n";
  out << "Build CPU    : " << build_processor() << "\n";

  return out.str();
}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3


