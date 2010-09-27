// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CodeProfiler_hpp
#define CF_Common_CodeProfiler_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/path.hpp>

#include "Common/ConcreteProvider.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  //////////////////////////////////////////////////////////////////////////////

  class Common_API CodeProfiler
  {
  public:
    typedef ConcreteProvider < CodeProfiler, 0 > PROVIDER;

    CodeProfiler();

    virtual ~CodeProfiler();

    static std::string type_name() { return "CodeProfiler"; }

    virtual void start_profiling() = 0;

    virtual void stop_profiling() = 0;

    virtual void set_file_path(const boost::filesystem::path & path) = 0;

  }; // class CodeProfiler

  //////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CodeProfiler_hpp
