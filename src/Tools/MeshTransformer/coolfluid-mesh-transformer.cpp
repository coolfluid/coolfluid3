// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CMesh.hpp"

#include "Tools/MeshTransformer/Transformer.hpp"
#include "Tools/Shell/Interpreter.hpp"
#include "Tools/Shell/BasicCommands.hpp"

using namespace boost;
using namespace boost::program_options;

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Tools::Shell;
using namespace CF::Tools::MeshTransformer;

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
  Core::instance().initiate(argc, argv);
  
	try
  {

    // --------------------------------------------------------

    ExceptionManager::instance().ExceptionDumps = false;
    ExceptionManager::instance().ExceptionAborts = false;

    // create mesh object
    CRoot::Ptr root = Core::instance().root();
    CMesh::Ptr mesh = root->create_component<CMesh>("mesh");

    // Initialize empty commands
    options_description desc;
    
    // Add basic commands to program
    desc.add(BasicCommands::description());
    
    // Add mesh transformer commands to program
    desc.add(Transformer::description());

    // Parse commands that are passed directly on the command line
    Interpreter shell(desc);
    shell.interpret(argc,argv);

    // --------------------------------------------------------

  }
  catch (boost::program_options::unknown_option &e) 
  {
    std::cerr << "error: " << e.what() << CFendl;
  }
  catch (boost::program_options::invalid_command_line_syntax &e)
  {
    std::cerr << "error: " << e.what() << CFendl;
  }
  catch (boost::program_options::validation_error &e)
  {
    std::cerr << "error: " << e.what() << CFendl;
  }
  catch(Exception & e)
  {
    std::cerr << e.what() << CFendl;
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
