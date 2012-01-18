// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <google/profiler.h>

#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "Tools/GooglePerfTools/LibGooglePerfTools.hpp"

#include "Tools/GooglePerfTools/GooglePerfProfiling.hpp"

using namespace cf3::common;
using namespace cf3::Tools::GooglePerfTools;

///////////////////////////////////////////////////////////////////////////////

ComponentBuilder < GooglePerfProfiling, CodeProfiler, LibGooglePerfTools > GooglePerfProfiling_Builder;

///////////////////////////////////////////////////////////////////////////////

GooglePerfProfiling::GooglePerfProfiling( const std::string& name) : CodeProfiler(name),
    m_profiling(false)
{    
  m_path = boost::filesystem::path("perftools-profile.pprof");
}

GooglePerfProfiling::~GooglePerfProfiling()
{

}

void GooglePerfProfiling::start_profiling()
{
  if( !m_profiling )
  {
    ProfilerStart(m_path.string().c_str());
    CFinfo <<  type_name() << ": Saving profile data to: " << m_path.string() << CFendl;
    m_profiling = true;
  }
  else
  {
    CFwarn << type_name() << ":  Was already profiling!" << CFendl;
  }
}

void GooglePerfProfiling::stop_profiling()
{
  ProfilerStop();
  m_profiling = false;
  CFinfo << type_name() << ": Stopping profiling" << CFendl;
}

void GooglePerfProfiling::set_file_path(const boost::filesystem::path & path)
{
  m_path = path;
}
