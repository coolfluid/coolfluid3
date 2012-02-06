// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_CodeProfiler_hpp
#define cf3_common_CodeProfiler_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/BoostFilesystem.hpp"

#include "common/Component.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

  //////////////////////////////////////////////////////////////////////////////

  class Common_API CodeProfiler : public Component
  {
  public:

    /// constructor
    CodeProfiler(const std::string& name);

    /// virtual destructor
    virtual ~CodeProfiler();

    /// this class type
    static std::string type_name() { return "CodeProfiler"; }

    /// start profiling
    virtual void start_profiling() = 0;

    /// sttop profiling
    virtual void stop_profiling() = 0;

    /// define the path of the file where to dump profiling information
    virtual void set_file_path(const boost::filesystem::path & path) = 0;

  }; // class CodeProfiler

  //////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_CodeProfiler_hpp
