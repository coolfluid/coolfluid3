//  (C) Copyright Jean-Daniel Michaud 2007. Permission to copy, use, modify, 
//  sell and distribute this software is granted provided this copyright notice 
//  appears in all copies. This software is provided "as is" without express or 
//  implied warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org/LICENSE_1_0.txt for licensing.
//  See http://code.google.com/p/clipo/ for library home page.

// Modified by Willem Deconinck for COOLFluiD
// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>
#include <fstream>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "Common/Exception.hpp"
#include "Common/Log.hpp"
#include "Common/Component.hpp"
#include "Common/Foreach.hpp"
#include "Tools/CommandLineInterpreter/CommandLineInterpreter.hpp"
#include "Tools/CommandLineInterpreter/BasicCommands.hpp"

using namespace CF::Common;
using namespace boost::program_options;

namespace CF {
namespace Tools { 
namespace CommandLineInterpreter {

////////////////////////////////////////////////////////////////////////////////

std::string default_prompt()
{
  return "["+BasicCommands::current_component->full_path().path()+"] ";
}

////////////////////////////////////////////////////////////////////////////////

std::string print_line(const std::string& line)
{
  CFinfo << line << CFendl;
  return line;
}

////////////////////////////////////////////////////////////////////////////////

CommandLineInterpreter::CommandLineInterpreter(const commands_description& desc) :
  m_prompt(default_prompt),
  m_desc("Basic Commands")
{ 
  set_description(desc);
}

/// Constructor taking description of commands and prompt function
CommandLineInterpreter::CommandLineInterpreter(const commands_description& desc, 
                       const prompt_function_pointer_t& prompt) :
  m_prompt(prompt),
  m_desc("Basic Commands")
{ 
  set_description(desc);
}

////////////////////////////////////////////////////////////////////////////////

void CommandLineInterpreter::set_description(const commands_description& desc)
{
  m_desc.add_options()
  ("help,h", "show help")
  ("interactive,i", "start shell")
  ("file,f", value< std::vector<std::string> >()->multitoken() , "execute coolfluid script file")
  ("save,s", value< std::string >()->implicit_value(std::string()), "save history")
  ("history", "show history")
  ("reset,r", "reset history")
  ;
  
  m_desc.add(desc);
}

////////////////////////////////////////////////////////////////////////////////

//Unshamefully copied from the split_winmain function developed by Vladimir Prus 
std::vector<std::string> CommandLineInterpreter::split_command_line(const std::string& input)
{
  std::vector<std::string> result;

  std::string::const_iterator i = input.begin(), e = input.end();
  for(;i != e; ++i)
    if (!isspace((unsigned char)*i))
      break;
 
  if (i != e) 
  {
    std::string current;
    bool inside_quoted = false;
    int backslash_count = 0;
    
    for(; i != e; ++i) 
    {
      if (*i == '"') 
      {
        // '"' preceded by even number (n) of backslashes generates
        // n/2 backslashes and is a quoted block delimiter
        if (backslash_count % 2 == 0) 
        {
          current.append(backslash_count / 2, '\\');
          inside_quoted = !inside_quoted;
          // '"' preceded by odd number (n) of backslashes generates
          // (n-1)/2 backslashes and is literal quote.
        } 
        else 
        {
          current.append(backslash_count / 2, '\\');                
          current += '"';                
        }
        backslash_count = 0;
      } 
      else if (*i == '\\') 
      {
        ++backslash_count;
      } else 
      {
        // Not quote or backslash. All accumulated backslashes should be
        // added
        if (backslash_count) 
        {
          current.append(backslash_count, '\\');
          backslash_count = 0;
        }
        if (isspace((unsigned char)*i) && !inside_quoted) 
        {
          // Space outside quoted section terminate the current argument
          result.push_back(current);
          current.resize(0);
          for(;i != e && isspace((unsigned char)*i); ++i)
            ;
          --i;
        } 
        else 
        {                  
          current += *i;
        }
      }
    }

    // If we have trailing backslashes, add them
    if (backslash_count)
      current.append(backslash_count, '\\');

    // If we have non-empty 'current' or we're still in quoted
    // section (even if 'current' is empty), add the last token.
    if (!current.empty() || inside_quoted)
      result.push_back(current);        
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////

void CommandLineInterpreter::handle_read_line(std::string line)
{
  std::vector<std::string> args;

  // huu, ugly...
  args = split_command_line(std::string("--") + line); 
  
  try
  {
    variables_map vm;
    store( command_line_parser(args).options(m_desc).run(), vm);
    notify(vm);
    
    m_history.push_back(line);
  }
  catch (boost::program_options::unknown_option &e) 
  {
    CFerror << "error: " << e.what() << CFendl;
  }
  catch (boost::program_options::invalid_command_line_syntax &e)
  {
    CFerror << "error: " << e.what() << CFendl;
  }
  catch (boost::program_options::validation_error &e)
  {
    CFerror << "error: " << e.what() << CFendl;
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
  
}

////////////////////////////////////////////////////////////////////////////////

void CommandLineInterpreter::interpret(int argc, char * argv[])
{
  parsed_options parsed = parse_command_line(argc, argv, m_desc);
  if (parsed.options.size())
  {
    typedef basic_option<char> Option;

    // notify only 1 program_option at a time to conserve
    // execution order
    parsed_options one_parsed_option(&m_desc);
    one_parsed_option.options.resize(1);
    boost_foreach(Option option, parsed.options)
    {
      one_parsed_option.options[0]=option;
      variables_map vm;
      store(one_parsed_option,vm);
      notify(vm);
    }
  }
  else
  {
    CFinfo << "coolfluid shell - command 'exit' to quit - command 'help' for help" << CFendl;
    interpret(std::cin);
  }
}

////////////////////////////////////////////////////////////////////////////////

void CommandLineInterpreter::notify(variables_map& vm)
{
  boost::program_options::notify(vm);
  
  if (vm.count("help"))
  {
    CFinfo << m_desc << CFendl;
  }

  if (vm.count("file"))
  {
    std::vector<std::string> files = vm["file"].as<std::vector<std::string> >();
    boost_foreach(std::string file_path, files)
    {
      std::ifstream myfile;          
      myfile.open (file_path.c_str());
      if (myfile.is_open()) 
      {
        interpret(myfile,print_line);
        myfile.close();            
      }
      else
      {
        throw FileSystemError(FromHere(), "File "+file_path+" not opened");
      }
    }
  }

  if (vm.count("save"))
  {
    std::string file_path = vm["save"].as<std::string>();
    if (file_path.empty())
      file_path = "history.cfscript";

    std::ofstream myfile;          
    myfile.open (file_path.c_str());
    if (myfile.is_open()) 
    {
      boost_foreach(const std::string& line, m_history)
        myfile << line << std::endl;
      myfile.close();            
    }
    else
    {
      throw FileSystemError(FromHere(), "File "+file_path+" not opened");
    }
  }

  if (vm.count("history"))
  {
    boost_foreach(const std::string& line, m_history)
      CFinfo << line << CFendl;
  }
  
  if (vm.count("reset"))
  {
    m_history.clear();
  }
  
  if (vm.count("interactive"))
  {
    CFinfo << "coolfluid shell - command 'exit' to quit - command 'help' for help" << CFendl;
    interpret(std::cin);        
  }
  
}

////////////////////////////////////////////////////////////////////////////////

void CommandLineInterpreter::interpret(std::istream &input_stream)
{
  std::string command;

  write_prompt();
  
  while (std::getline(input_stream, command, '\n'))
  {
    handle_read_line(command);
    write_prompt();
  }
}

////////////////////////////////////////////////////////////////////////////////

void CommandLineInterpreter::interpret(std::istream &input_stream, readline_function_pointer_t f)
{
  std::string command;

  while (std::getline(input_stream, command, '\n'))
  {
    command.erase(boost::algorithm::find_first(command,"#").begin(),command.end());
    boost::algorithm::trim(command); // remove leading and trailing white spaces
    if (!command.empty())
    {
      write_prompt();
      handle_read_line(f(command));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
  
void CommandLineInterpreter::write_prompt()
{
  CFinfo << m_prompt() << CFflush;
}

////////////////////////////////////////////////////////////////////////////////

} // CommandLineInterpreter
} // Tools
} // CF
