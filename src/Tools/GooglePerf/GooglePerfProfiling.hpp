// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Tools_GooglePerf_GooglePerfProfiling_hpp
#define CF_Tools_GooglePerf_GooglePerfProfiling_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CodeProfiler.hpp"

#include "Tools/GooglePerf/LibGooglePerfTools.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace GooglePerf {

////////////////////////////////////////////////////////////////////////////////

#error "Make CodeProfiler into a component"

class GooglePerfTools_API GooglePerfProfiling : public CF::Common::CodeProfiler
{
  public:
    GooglePerfProfiling();

    static std::string type_name() { return "GooglePerfProfiling"; }

    virtual ~GooglePerfProfiling();

    virtual void start_profiling();

    virtual void stop_profiling();

    virtual void set_file_path(const boost::filesystem::path & path);

  private:

    bool m_profiling;

    boost::filesystem::path m_path;

};

////////////////////////////////////////////////////////////////////////////////

} // GooglePerf
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_GooglePerf_GooglePerfProfiling_hpp
