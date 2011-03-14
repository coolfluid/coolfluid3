// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/program_options.hpp>
#include "Common/CF.hpp"
#include "Common/Exception.hpp"
#include "Common/Log.hpp"
#include "Common/Core.hpp"

#include "Tools/Shell/Interpreter.hpp"
#include "Tools/Shell/BasicCommands.hpp"

using namespace boost::program_options;

using namespace CF;
using namespace CF::Common;
using namespace CF::Tools::Shell;

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
  Core::instance().initiate(argc, argv);

  try
  {

    // --------------------------------------------------------

    ExceptionManager::instance().ExceptionOutputs = false;
    ExceptionManager::instance().ExceptionDumps   = false;
    ExceptionManager::instance().ExceptionAborts  = false;
    AssertionManager::instance().AssertionThrows  = true;

    // Initialize empty commands
    options_description desc;

    // Add basic commands to program
    desc.add(BasicCommands::description());

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
