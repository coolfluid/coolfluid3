// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/BuildInfo.hpp"
#include "Common/CJournal.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/SF/Types.hpp"
#include "Mesh/CTable.hpp"

using namespace boost;
using namespace boost::program_options;

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;

/////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
  Core::instance().initiate(argc, argv);

  try
  {
    std::string journal_file;

    // parse command line options

    options_description desc("General Options");
    desc.add_options()
    ("help,h",   value<std::string>()->implicit_value(std::string()), "show this help")
    ("scase,s",  value<std::vector<std::string> >()->multitoken(),    "list of scripts to execute in order")
    ("exec_journal", value<std::string>(&journal_file)->implicit_value(std::string()), "load and execute a journal")
    ("version,v", "show version")
    ;
    variables_map vm;
    parsed_options parsed = parse_command_line(argc, argv, desc);
    store(parsed, vm);
    notify(vm);

    if (vm.count("version"))
      CFinfo << Core::instance().build_info()->version_header() << CFendl;

    // show help if asked or no args given
    if (vm.count("help") || vm.size()==0)
    {
      // Default help
      CFinfo << "CF3 batch command\n" << CFendl;
      CFinfo << "Usage: " << argv[0] << " [options]\n" << CFendl;
      CFinfo << desc << CFendl;

      Core::instance().terminate();
      return 0;
    }

    if(vm.count("exec_journal") == 1)
    {
      try
      {
        CJournal::Ptr journal(new CJournal("Journal"));

        Core::instance().root()->get_child_ptr("Tools")->add_component(journal);

        journal->execute_signals(journal_file);

        Core::instance().root()->save_tree_to("./my-tree.xml");

        return 0;
      }
      catch(Exception & e)
      {
        CFerror << e.what() << CFendl;
        return 1;
      }
    }
  }
  catch ( std::exception& ex )
  {
    CFerror << "Unhandled exception: " << ex.what() << CFendl;
  }
  catch ( ... )
  {
    CFerror << "Detected unknown exception" << CFendl;
  }

  Core::instance().terminate();

  return 0;
}


///////////////////////////////////////////////////////////////////////////////


