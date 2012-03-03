// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_common_NoProfiling_hpp
#define cf3_common_NoProfiling_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CodeProfiler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

  ////////////////////////////////////////////////////////////////////////////////

  class Common_API NoProfiling : public CodeProfiler
  {
  public:

    /// constructor
    NoProfiling(const std::string& name);

    virtual ~NoProfiling();

    static std::string type_name() { return "NoProfiling"; }

    virtual void start_profiling() {}

    virtual void stop_profiling() {}

  }; // class NoProfiling

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_NoProfiling_hpp
