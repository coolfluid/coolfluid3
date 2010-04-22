#include <google/profiler.h>

#include "Tools/GooglePerf/GooglePerfTools.hpp"

#include "Common/DirPaths.hpp"
#include "Common/Log.hpp"
#include "Common/ObjectProvider.hpp"

namespace CF {
namespace Tools {
namespace GooglePerf {

Common::ObjectProvider < GooglePerfToolsModule,
                         GooglePerfToolsModule,
                         GooglePerfToolsModule >
aCPUProfiler_Provider ( "GooglePerfTools" );

GooglePerfToolsModule::GooglePerfToolsModule()
{
  m_init = false;
}

void GooglePerfToolsModule::initiate() {
  if(!isInitialized()) {
    m_init = true;
    boost::filesystem::path prof_path = Common::DirPaths::getInstance().getResultsDir() / boost::filesystem::path("perftools-profile.pprof");
    CFinfo <<  getModuleName() << ": Saving profile data to: "  << prof_path.native_file_string() << "\n";
    ProfilerStart(prof_path.native_file_string().c_str());
  }
}

void GooglePerfToolsModule::terminate()
{
  CFinfo << getModuleName() << ": Stopping profiling\n";
  ProfilerStop();
}

} // GooglePerfTools
} // Tools
} // CF
