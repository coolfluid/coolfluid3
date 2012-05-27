// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CodeProfiler.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"

namespace cf3 {
namespace common {

CodeProfiler::CodeProfiler(const std::string& name) : Component (name)
{
  options().add("file_path", URI("profile_file.txt", cf3::common::URI::Scheme::FILE))
    .pretty_name("File Path")
    .description("Path to the file where the profile data should be stored");
    
  regist_signal( "start_profiling" )
    .connect( boost::bind( &CodeProfiler::signal_start_profiling, this, _1 ) )
    .description("Start the profiler")
    .pretty_name("Start Profiling");
    
  regist_signal( "stop_profiling" )
    .connect( boost::bind( &CodeProfiler::signal_stop_profiling, this, _1 ) )
    .description("Stop the profiler")
    .pretty_name("Stop Profiling");
}

void CodeProfiler::signal_start_profiling(SignalArgs& args)
{
  start_profiling();
}

void CodeProfiler::signal_stop_profiling(SignalArgs& args)
{
  stop_profiling();
}


CodeProfiler::~CodeProfiler()
{
}

} // common
} // cf3

