#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include "Tools/GooglePerf/ProfiledTestFixture.hpp"
#include "Tools/GooglePerf/GooglePerfTools.hpp"


////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace GooglePerf {

#ifdef CF_BUILD_GooglePerfTools

////////////////////////////////////////////////////////////////////////////////

ProfiledTestFixture::ProfiledTestFixture() : m_profiler(ModuleRegister<GooglePerfToolsModule>::getInstance()) {
  int argc = boost::unit_test::framework::master_test_suite().argc;
  char** argv = boost::unit_test::framework::master_test_suite().argv;

  if(argc == 2) {
    m_profiler.setFilePath(boost::filesystem::path(argv[1]));
  } else {
    boost::filesystem::path commandPath(argv[0]);
    boost::char_separator<char> separator(".");
    boost::tokenizer<boost::char_separator<char> > tokenizer(commandPath.leaf(), separator);
    m_prefix = *tokenizer.begin();
    m_profile_dir = commandPath.parent_path();
    m_command = std::string(argv[0]);
  }
}

////////////////////////////////////////////////////////////////////////////////

void ProfiledTestFixture::startProfiling(const std::string& ProfileName) {
  m_current_filename = m_prefix + (ProfileName.size() ? "-" : "") + ProfileName + ".pprof";
  m_profiler.setFilePath(boost::filesystem::path(m_profile_dir / m_current_filename));
  m_profiler.initiate();
}

////////////////////////////////////////////////////////////////////////////////

void ProfiledTestFixture::stopProfiling() {
  m_profiler.terminate();
}

#endif

////////////////////////////////////////////////////////////////////////////////

void ProfiledTestFixture::procesProfilingFile() {
  cf_assert(boost::algorithm::ends_with(m_current_filename, ".pprof"));
  boost::filesystem::path infile(m_profile_dir / m_current_filename);
  boost::filesystem::path outfile(m_profile_dir / boost::algorithm::erase_last_copy(m_current_filename, ".pprof"));
  std::string pprof_command(CF_PPROF_COMMAND);
  if(pprof_command.size()) {
    try {
      std::string pprof_line(pprof_command + " --dot " + m_command + " " + infile.file_string() + " > " + outfile.file_string() + ".dot");
      std::string dot_line(std::string(CF_DOT_COMMAND) + " -Tsvg " + outfile.file_string() + ".dot > " + outfile.file_string() + ".svg");
      OSystem::getInstance().executeCommand(pprof_line);
      OSystem::getInstance().executeCommand(dot_line);
    } catch(OSystemError& E) {
      // Fail softly and inform the user, since a profiling error is not fatal.
      CFwarn << "Error processing profile file: " << E.what() << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // GooglePerf
} // Tools
} // CF
