// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Common_NoProfiling_hpp
#define CF_Common_NoProfiling_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CodeProfiler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  ////////////////////////////////////////////////////////////////////////////////

  class Common_API NoProfiling : public CodeProfiler
  {
  public:

    typedef boost::shared_ptr<NoProfiling> Ptr;
    typedef boost::shared_ptr<NoProfiling const> ConstPtr;

    /// constructor
    NoProfiling(const std::string& name);

    virtual ~NoProfiling();

    static std::string type_name() { return "NoProfiling"; }

    virtual void start_profiling() {}

    virtual void stop_profiling() {}

    virtual void set_file_path(const boost::filesystem::path & path) {}

  }; // class NoProfiling

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_NoProfiling_hpp
