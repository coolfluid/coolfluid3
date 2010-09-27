// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <google/profiler.h>

#include "Common/ObjectProvider.hpp"
#include "Common/DirPaths.hpp"

#include "Tools/GooglePerf/LibGooglePerfTools.hpp"

#include "Tools/GooglePerf/GooglePerfProfiling.hpp"

using namespace CF::Common;
using namespace CF::Tools::GooglePerf;

///////////////////////////////////////////////////////////////////////////////

ObjectProvider < GooglePerfProfiling, CodeProfiler, LibGooglePerfTools, NB_ARGS_0 >
GooglePerfProfiling_Provider ( GooglePerfProfiling::type_name() );

///////////////////////////////////////////////////////////////////////////////

GooglePerfProfiling::GooglePerfProfiling() :
    m_profiling(false)
{
  m_path = Common::DirPaths::instance().getResultsDir() / boost::filesystem::path("perftools-profile.pprof");
}

GooglePerfProfiling::~GooglePerfProfiling()
{

}

void GooglePerfProfiling::start_profiling()
{
  if( !m_profiling )
  {
    ProfilerStart(m_path.native_file_string().c_str());
    CFinfo <<  type_name() << ": Saving profile data to: " <<
        m_path.native_file_string() << CFendl;
    m_profiling = true;
  }
  else
  {
    CFwarn << type_name() << "Was already profiling!" << CFendl;
  }
}

void GooglePerfProfiling::stop_profiling()
{
  ProfilerStop();
  CFinfo << type_name() << ": Stopping profiling" << CFendl;
}

void GooglePerfProfiling::set_file_path(const boost::filesystem::path & path)
{
  m_path = path;
}
