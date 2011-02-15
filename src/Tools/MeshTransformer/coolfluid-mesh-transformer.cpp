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

#include "Tools/MeshTransformer/Transformer.hpp"

using namespace boost;
using namespace boost::program_options;

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Tools::MeshTransformer;

int main(int argc, char * argv[])
{
  Core::instance().initiate(argc, argv);

  ExceptionManager::instance().ExceptionDumps = true;
	ExceptionManager::instance().ExceptionAborts = true;

	try
  {

    Transformer transformer;
    
		options_description desc("General Options");
		desc.add_options()
		("help,h", value<std::string>()->implicit_value(std::string()) , "this help if no arg, or more detailed help of submodule")
		("input,i" , value<std::vector<std::string> >()->multitoken(), "input file(s)")
		("output,o", value<std::vector<std::string> >()->multitoken(), "output file(s)")
		("transform,t", value<std::vector<std::string> >()->multitoken(), "transformations")
		("dryrun,d", "dry run")
		("version,v", "show version")
		;
		variables_map vm;
		parsed_options parsed = parse_command_line(argc, argv, desc);
		store(parsed, vm);
		notify(vm);

    if (vm.size() == 0)
    {
      // CFinfo << "Started in interactive mode. ( type \"quit\" to exit, \"help\" for available commands)" << CFendl;
      // std::string command_line = std::string();
      // std::string command_line_args = std::string();
      // boost::regex quit  ("(q(uit)?)|exit" , boost::regex::perl|boost::regex::icase);
      // 
      // while ( boost::regex_match(command_line_args,quit) == false)
      // {
      //   CFinfo << " > " << CFflush;
      //   getline(std::cin,command_line_args);
      //   command_line = "coolfluid-mesh-transformer "+command_line_args;
      //   std::vector<std::string> split = boost::program_options::split_unix(command_line);
      //   int argc_inter = split.size();        
      //   std::vector<char*> argv_inter;
      // 
      //   boost_foreach(std::string arg, split)
      //   {
      //     char p[256];
      //     strcpy(p,arg.c_str());
      //     argv_inter.push_back(p);
      //     CFinfo << "arg = " << arg << CFendl;
      //   }
      //   parsed_options interactive_parsed = parse_command_line(argc_inter, &argv_inter[0], desc);
      //   
      //   transformer.command(interactive_parsed);
      // }
    }
    else
    {
      transformer.command(parsed);
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
