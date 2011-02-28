// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Foreach.hpp"
#include "Mesh/CMesh.hpp"

#include "Tools/MeshTransformer/Transformer.hpp"
#include "Tools/CommandLineInterpreter/CommandLineInterpreter.hpp"
#include "Tools/CommandLineInterpreter/BasicCommands.hpp"

using namespace boost;
using namespace boost::program_options;

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Tools::CommandLineInterpreter;
using namespace CF::Tools::MeshTransformer;

////////////////////////////////////////////////////////////////////////////////

std::string pwd_prompt()
{
  return "["+BasicCommands::current_component->full_path().path()+"] ";
}

////////////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv[])
{
  Core::instance().initiate(argc, argv);
  
	try
  {
////////////////////////////////////////////////////////////////////////////////
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

    // Parse commands passed directly on the command line
    parsed_options parsed = parse_command_line(argc, argv, desc);
    if (parsed.options.size())
    {
      typedef basic_option<char> Option;

      // notify only 1 program_option at a time to conserve
      // execution order
      parsed_options one_parsed_option(parsed.description);
      one_parsed_option.options.resize(1);
      boost_foreach(Option option, parsed.options)
      {
        one_parsed_option.options[0]=option;
        boost::program_options::variables_map vm;
        boost::program_options::store(one_parsed_option,vm);
        boost::program_options::notify(vm);
      }
    }
    
    // Start interactive shell
    CFinfo << "\ncoolfluid shell - command 'exit' to quit - command 'help' for help" << CFendl;
    CommandLineInterpreter cli(desc , &pwd_prompt);
    cli.interpret(std::cin);

////////////////////////////////////////////////////////////////////////////////
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
