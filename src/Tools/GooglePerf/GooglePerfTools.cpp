#include <google/profiler.h>

#include "Tools/GooglePerf/GooglePerfTools.hpp"

#include "Common/DirPaths.hpp"
#include "Common/Log.hpp"
#include "Common/ObjectProvider.hpp"

namespace CF {
namespace Tools {
namespace GooglePerf {

GooglePerfToolsModule::GooglePerfToolsModule()
{
  m_init = false;
  m_path = Common::DirPaths::getInstance().getResultsDir() / boost::filesystem::path("perftools-profile.pprof");
}

void GooglePerfToolsModule::initiate() {
  if(!isInitialized()) {
    m_init = true;
    CFinfo <<  getModuleName() << ": Saving profile data to: "  << m_path.native_file_string() << "\n";
    ProfilerStart(m_path.native_file_string().c_str());
  } else {
    CFwarn << getModuleName() << "Was already profiling!\n";
  }
}

void GooglePerfToolsModule::terminate() {
  CFinfo << getModuleName() << ": Stopping profiling\n";
  ProfilerStop();
  m_init = false;
}

void GooglePerfToolsModule::setFilePath(const boost::filesystem::path& path) {
  m_path = path;
}

} // GooglePerfTools
} // Tools
} // CF
