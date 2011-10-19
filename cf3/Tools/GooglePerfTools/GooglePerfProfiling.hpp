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

class GooglePerfTools_API GooglePerfProfiling : public Common::CodeProfiler
{
public: // typedefs

  /// pointer to this type
  typedef boost::shared_ptr<GooglePerfProfiling> Ptr;
  typedef boost::shared_ptr<GooglePerfProfiling const> ConstPtr;

public: // functions

    GooglePerfProfiling( const std::string& name );

    virtual ~GooglePerfProfiling();

    static std::string type_name() { return "GooglePerfProfiling"; }

    virtual void start_profiling();

    virtual void stop_profiling();

    virtual void set_file_path(const boost::filesystem::path & path);

private:

    bool m_profiling;

    boost::filesystem::path m_path;

};

////////////////////////////////////////////////////////////////////////////////

} // GooglePerfTools
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_GooglePerf_GooglePerfProfiling_hpp
