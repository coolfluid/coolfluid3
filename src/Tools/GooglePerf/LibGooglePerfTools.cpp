// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <google/profiler.h>

#include "Tools/GooglePerf/LibGooglePerfTools.hpp"

#include "Common/CoreEnv.hpp"
#include "Common/DirPaths.hpp"
#include "Common/Log.hpp"
#include "Common/ObjectProvider.hpp"

namespace CF {
namespace Tools {
namespace GooglePerf {

  CF::Common::ForceLibRegist<LibGooglePerfTools> libGooglePerfTools;


LibGooglePerfTools::LibGooglePerfTools()
{
  m_init = false;
  m_path = Common::DirPaths::instance().getResultsDir() / boost::filesystem::path("perftools-profile.pprof");
}

void LibGooglePerfTools::initiate() {
  if(!isInitialized()) {
    m_init = true;
    CFinfo <<  library_name() << ": Saving profile data to: "  << m_path.native_file_string() << CFendl;
    ProfilerStart(m_path.native_file_string().c_str());
  } else {
    CFwarn << library_name() << "Was already profiling!" << CFendl;
  }
}

void LibGooglePerfTools::terminate() {
  CFinfo << library_name() << ": Stopping profiling" << CFendl;
  ProfilerStop();
  m_init = false;
}

void LibGooglePerfTools::setFilePath(const boost::filesystem::path& path) {
  m_path = path;
}

} // GooglePerf
} // Tools
} // CF
