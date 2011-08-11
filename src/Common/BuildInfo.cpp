// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "coolfluid-svn-revision.hpp"

#include <sstream>

#include "Common/BuildInfo.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::build_type () const
{
  return CF_BUILD_TYPE;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::svn_version () const
{
  return CF_SVNVERSION;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::release_version () const
{
  return CF_VERSION_STR;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::kernel_version () const
{
  return CF_KERNEL_VERSION_STR;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::build_processor () const
{
  return CF_BUILD_PROCESSOR;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::build_tool () const
{
  std::string ret;
#ifdef CF_CMAKE_VERSION
  ret += "CMake ";
  ret += CF_CMAKE_VERSION;
#else
  ret += "UNKNOWN";
#endif
  return ret;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::os_name() const
{
  return CF_OS_NAME;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::os_long_name() const
{
  return CF_OS_LONGNAME;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::os_version() const
{
  return CF_OS_VERSION;
}

////////////////////////////////////////////////////////////////////////////////

std::string BuildInfo::os_bits() const
{
  return CF_OS_BITS;
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

} // Common
} // CF


