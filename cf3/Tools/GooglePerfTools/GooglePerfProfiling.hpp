// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Tools_GooglePerf_GooglePerfProfiling_hpp
#define CF_Tools_GooglePerf_GooglePerfProfiling_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CodeProfiler.hpp"

#include "Tools/GooglePerfTools/LibGooglePerfTools.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {
namespace GooglePerfTools {

////////////////////////////////////////////////////////////////////////////////

class GooglePerfTools_API GooglePerfProfiling : public common::CodeProfiler
{
public: // typedefs

  /// pointer to this type
  
  

public: // functions

    GooglePerfProfiling( const std::string& name );

    virtual ~GooglePerfProfiling();

    static std::string type_name() { return "GooglePerfProfiling"; }

    virtual void start_profiling();

    virtual void stop_profiling();

private:

    bool m_profiling;
};

////////////////////////////////////////////////////////////////////////////////

} // GooglePerfTools
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_GooglePerf_GooglePerfProfiling_hpp
