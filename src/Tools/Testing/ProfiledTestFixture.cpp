// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <fstream>

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include "Common/OSystem.hpp"
#include "Common/MPI/PEInterface.hpp"

#include "Tools/Testing/ProfiledTestFixture.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace Testing {

////////////////////////////////////////////////////////////////////////////////

ProfiledTestFixture::ProfiledTestFixture() :
  m_using_google_perf(Factory<CodeProfiler>::instance().exists( "GooglePerfProfiling" ))

{

  if(m_using_google_perf)
    OSystem::instance().set_profiler("GooglePerfProfiling");

  char** argv = boost::unit_test::framework::master_test_suite().argv;

  boost::filesystem::path commandPath(argv[0]);
  boost::char_separator<char> separator(".");
  boost::tokenizer<boost::char_separator<char> > tokenizer(commandPath.leaf(), separator);
  m_prefix = *tokenizer.begin();
  m_profile_dir = commandPath.parent_path();
  m_command = std::string(argv[0]);

  test_unit_start(boost::unit_test::framework::current_test_case());
}

////////////////////////////////////////////////////////////////////////////////

ProfiledTestFixture::~ProfiledTestFixture() {
  test_unit_finish(boost::unit_test::framework::current_test_case());
}

////////////////////////////////////////////////////////////////////////////////

void ProfiledTestFixture::test_unit_start( boost::unit_test::test_unit const& unit ) {
  std::stringstream job_suffix;
  if(PEInterface::instance().is_init())
  {
    job_suffix << "-" << PEInterface::instance().rank();
  }

  if(m_using_google_perf)
  {
    m_current_filename = m_prefix + "-" + unit.p_name.get() + job_suffix.str() + ".pprof";
    OSystem::instance().profiler()->set_file_path(boost::filesystem::path(m_profile_dir / m_current_filename));
    OSystem::instance().profiler()->start_profiling();
  }
}

////////////////////////////////////////////////////////////////////////////////

void ProfiledTestFixture::test_unit_finish( boost::unit_test::test_unit const& unit ) {
  if(m_using_google_perf)
  {
    OSystem::instance().profiler()->stop_profiling();
    if(PEInterface::instance().rank() > 0)
      return;
    cf_assert(boost::algorithm::ends_with(m_current_filename, ".pprof"));
    boost::filesystem::path infile(m_profile_dir / m_current_filename);
    std::string basename = boost::algorithm::erase_last_copy(m_current_filename, ".pprof");
    boost::filesystem::path outfile(m_profile_dir / basename);
    std::string pprof_command(CF_PPROF_COMMAND);
    if(pprof_command.size()) { // process the profile file, if the command exists
      try {
        // note: graph output is too heavy for the dashboard
        //std::string pprof_line(pprof_command + " --dot " + m_command + " " + infile.file_string() + " > " + outfile.file_string() + ".dot");
        //std::string dot_line(std::string(CF_DOT_COMMAND) + " -Tpng " + outfile.file_string() + ".dot > " + outfile.file_string() + ".png");
        std::string pprof_line(pprof_command + " --text " + m_command + " " + infile.file_string() + " | head -n 10 > " + outfile.file_string() + ".txt");
        OSystem::instance().executeCommand(pprof_line);
        //OSystem::instance().executeCommand(dot_line);

        // Read the output text file for dasboard output
        std::string profile; // We will read the contents here.
        std::fstream file((outfile.file_string() + ".txt").c_str());
        file.unsetf(std::ios::skipws); // No white space skipping!
        std::copy(
            std::istream_iterator<char>(file),
            std::istream_iterator<char>(),
            std::back_inserter(profile));
        // Output a CDash reference to the generated profiling graph
        std::cout << "<DartMeasurement name=\""<< unit.p_name.get() << " profile data\" type=\"text/plain\"><![CDATA[<html><body><pre>" + profile + "</pre></body></html>]]></DartMeasurement>" << std::endl;
        //CFinfo << "<DartMeasurementFile name=\""<< basename << " profile graph\" type=\"image/png\">" + outfile.file_string() + ".png</DartMeasurementFile>\n";
      } catch(OSystemError& E) {
        // Fail softly and inform the user, since a profiling error is not fatal.
        std::cout << "Error processing profile file: " << E.what() << CFendl;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // Testing
} // Tools
} // CF

