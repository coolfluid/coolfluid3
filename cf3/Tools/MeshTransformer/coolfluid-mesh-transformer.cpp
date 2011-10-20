// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/CRoot.hpp"
#include "common/Foreach.hpp"

#include "Mesh/CMesh.hpp"

#include "Tools/MeshTransformer/Transformer.hpp"
#include "Tools/Shell/Interpreter.hpp"
#include "Tools/Shell/BasicCommands.hpp"

using namespace boost;
using namespace boost::program_options;

using namespace cf3;
using namespace cf3::common;
using namespace cf3::Mesh;
using namespace cf3::Tools::Shell;
using namespace cf3::Tools::MeshTransformer;

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
  Core::instance().initiate(argc, argv);
  
	try
  {

    // --------------------------------------------------------

    ExceptionManager::instance().ExceptionDumps = false;
    ExceptionManager::instance().ExceptionOutputs = false;
    ExceptionManager::instance().ExceptionAborts = false;

    // create mesh object
    CRoot& root = Core::instance().root();
    CMesh::Ptr mesh = root.create_component_ptr<CMesh>("mesh");

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
  catch(Exception & e)
  {
    std::cerr << e.what() << CFendl;
  }
  catch ( std::exception& ex )
  {
    CFerror << "Unhandled exception caught at " << FromHere().short_str() << ": " << ex.what() << CFendl;
  }
  catch ( ... )
  {
    CFerror << "Detected unknown exception" << CFendl;
  }
	
  Core::instance().terminate();
	
  return 0;
}