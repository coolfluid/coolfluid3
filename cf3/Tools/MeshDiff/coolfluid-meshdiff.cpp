// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/program_options.hpp>
#include "common/CF.hpp"
#include "common/Exception.hpp"
#include "common/Log.hpp"
#include "common/Core.hpp"

#include "Tools/Shell/Interpreter.hpp"
#include "Tools/Shell/BasicCommands.hpp"

#include "Tools/MeshDiff/Commands.hpp"

using namespace boost::program_options;

using namespace cf3;
using namespace cf3::common;
using namespace cf3::Tools::Shell;
using namespace cf3::Tools::MeshDiff;

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
  Core::instance().initiate(argc, argv);

  try
  {

    // --------------------------------------------------------

    ExceptionManager::instance().ExceptionOutputs = false;
    ExceptionManager::instance().ExceptionDumps   = true;
    ExceptionManager::instance().ExceptionAborts  = false;
    AssertionManager::instance().AssertionThrows  = true;

    // Initialize empty commands
    options_description desc;

    // Add basic commands to program
    desc.add(BasicCommands::description());
    desc.add(Commands::description());

    // Parse commands that are passed directly on the command line
    Interpreter shell(desc);
    shell.interpret(argc,argv);

    // --------------------------------------------------------

  }
  catch(Exception & e)
  {
    CFerror << e.what() << CFendl;
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
