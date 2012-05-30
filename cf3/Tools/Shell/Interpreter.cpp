//  (C) Copyright Jean-Daniel Michaud 2007. Permission to copy, use, modify,
//  sell and distribute this software is granted provided this copyright notice
//  appears in all copies. This software is provided "as is" without express or
//  implied warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org/LICENSE_1_0.txt for licensing.
//  See http://code.google.com/p/clipo/ for library home page.

// Modified by Willem Deconinck for COOLFluiD
// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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
#include <boost/algorithm/string/replace.hpp>
#include <boost/tokenizer.hpp>

#include "common/Exception.hpp"
#include "common/Log.hpp"
#include "common/Component.hpp"
#include "common/Foreach.hpp"
#include "Tools/Shell/Interpreter.hpp"
#include "Tools/Shell/BasicCommands.hpp"

using namespace cf3::common;
using namespace boost::program_options;

namespace cf3 {
namespace Tools {
namespace Shell {

////////////////////////////////////////////////////////////////////////////////

std::string default_prompt()
{
  return "["+BasicCommands::current_component->uri().path()+"] ";
}

////////////////////////////////////////////////////////////////////////////////

std::string print_line(const std::string& line)
{
  CFinfo << line << CFendl;
  return line;
}

////////////////////////////////////////////////////////////////////////////////

Interpreter::Interpreter(const commands_description& desc) :
  m_prompt(default_prompt),
  m_desc("Basic Commands")
{
  description(desc);

  m_handle_unrecognized_commands.push_back(&BasicCommands::unrecognized);
  m_handle_unrecognized_commands.push_back(boost::bind(&Interpreter::interpret_alias,this,_1));
}

/// Constructor taking description of commands and prompt function
Interpreter::Interpreter(const commands_description& desc,
                       const prompt_function_pointer_t& prompt) :
  m_prompt(prompt),
  m_desc("Basic Commands")
{
  description(desc);
}

////////////////////////////////////////////////////////////////////////////////

void Interpreter::description(const commands_description& desc)
{
  m_desc.add_options()
  //("help,h", value< std::vector<std::string> >()->multitoken()->zero_tokens(), "show help")
  ("interactive,i", value< std::vector<std::string> >()->multitoken()->zero_tokens(), "start shell")
  ("file,f", value< std::vector<std::string> >()->multitoken() , "execute coolfluid script file")
  ("save,s", value< std::string >()->implicit_value(std::string()), "save history")
  ("alias", value<std::string>()->notifier(boost::bind(&Interpreter::alias, this, _1)))
  ("history", value< std::vector<std::string> >()->multitoken()->zero_tokens(), "show history")
  ("reset,r", value< std::vector<std::string> >()->multitoken()->zero_tokens(), "reset history")
  ;

  m_desc.add(desc);
}

////////////////////////////////////////////////////////////////////////////////

void Interpreter::interpret_alias(std::vector<std::string>& unrecognized_commands)
{
  Uint cnt(0);
  boost_foreach(std::string& command, unrecognized_commands)
  {
    std::string alias_str = command;
    std::string arg_str = "";
    alias_str.erase( boost::algorithm::find_first(alias_str," ").begin(),alias_str.end());
    if (m_alias.find(alias_str) != m_alias.end())
    {
      if (alias_str != command)
        arg_str = std::string( boost::algorithm::find_first(command," ").begin(),command.end());
      unrecognized_commands.erase(unrecognized_commands.begin()+cnt);
      handle_read_line(m_alias[alias_str]+arg_str);
    }
    ++cnt;
  }
}

////////////////////////////////////////////////////////////////////////////////

void Interpreter::alias(const std::string& arg)
{
  std::string alias_str = arg;
  std::string command_str = arg;
  alias_str.erase( boost::algorithm::find_first(alias_str,"=").begin(),alias_str.end());
  command_str.erase( command_str.begin(), boost::algorithm::find_first(command_str,"=").begin()+1);
  m_alias[alias_str]=command_str;
}

////////////////////////////////////////////////////////////////////////////////

//Unshamefully copied from the split_winmain function developed by Vladimir Prus
std::vector<std::string> Interpreter::split_command_line(const std::string& input)
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

void Interpreter::handle_read_line(std::string line)
{

  line = BasicCommands::filter_env_vars(line);

  std::vector<std::string> args;

  args = split_command_line(std::string("--") + line);

  try
  {
    parsed_options parsed = command_line_parser(args).options(m_desc).allow_unregistered().run();
    variables_map vm;
    store( parsed , vm);
    notify(vm);
    std::vector<std::string> unrecognized_commands_no_args = collect_unrecognized( parsed.options , include_positional);
    std::vector<std::string> unrecognized_commands;
    boost_foreach(std::string& command, unrecognized_commands_no_args)
    {
      if ( std::string(command.begin(),command.begin()+2) == "--")
        unrecognized_commands.push_back(std::string(command.begin()+2,command.end()));
      else if ( std::string(command.begin(),command.begin()+1) == "-")
        throw boost::program_options::unknown_option(" argument cannot start with '-'");
      else
        unrecognized_commands.back() = unrecognized_commands.back() + " " + command;
    }
    boost_foreach(const unrecognized_commands_handler_t& handle, m_handle_unrecognized_commands )
    {
      if (unrecognized_commands.size())
      {
        handle(unrecognized_commands);
      }
    }
    if (unrecognized_commands.size())
      throw boost::program_options::unknown_option("");
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

void Interpreter::interpret(int argc, char * argv[])
{
  try
  {
    // parsed_options parsed = parse_command_line(argc, argv, m_desc);
    parsed_options parsed = command_line_parser(argc,argv).options(m_desc).allow_unregistered().run();

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
        std::vector<std::string> unrecognized_commands = collect_unrecognized( parsed.options , include_positional);
        boost_foreach(std::string& command, unrecognized_commands)
        {
          // remove "--" from command
          command.erase(command.begin(),command.begin()+2);
        }
        boost_foreach(const unrecognized_commands_handler_t& handle, m_handle_unrecognized_commands )
        {
          if (unrecognized_commands.size())
          {
            handle(unrecognized_commands);
          }
        }
        if (unrecognized_commands.size())
          throw boost::program_options::unknown_option("");

      }
    }
    else
    {
      CFinfo << "coolfluid shell - command 'exit' to quit - command 'help' for help" << CFendl;
      interpret(std::cin);
    }
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

void Interpreter::notify(variables_map& vm)
{
  boost::program_options::notify(vm);

//  if (vm.count("help"))
//  {
//    CFinfo << m_desc << CFendl;
//  }

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

void Interpreter::interpret(std::istream &input_stream)
{
  std::string command;

  write_prompt();

  while (std::getline(input_stream, command, '\n'))
  {
    bool multi_line = boost::algorithm::iends_with(command, "\\");
    while ( multi_line )
    {
      boost::algorithm::erase_last(command,"\\");
      std::string more;
      std::getline(input_stream, more, '\n');
      command += more;
      multi_line = boost::algorithm::iends_with(command, "\\");
    }

    handle_read_line(command);
    write_prompt();
  }
}

////////////////////////////////////////////////////////////////////////////////

void Interpreter::interpret(std::istream &input_stream, readline_function_pointer_t f)
{
  std::string command;

  while (std::getline(input_stream, command, '\n'))
  {
    bool multi_line = boost::algorithm::iends_with(command, "\\");
    while ( multi_line )
    {
      boost::algorithm::erase_last(command,"\\");
      std::string more;
      std::getline(input_stream, more, '\n');
      command += more;
      multi_line = boost::algorithm::iends_with(command, "\\");
    }

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

void Interpreter::write_prompt()
{
  CFinfo << m_prompt() << CFflush;
}

////////////////////////////////////////////////////////////////////////////////

Command::Command(const std::string& command, const std::string& description, commands_description& commands) : m_commands(commands)
{
  add_options()
    (command.c_str(), value< std::vector<std::string> >()->notifier(boost::bind(&Command::handle, this, _1))->multitoken()->zero_tokens(), description.c_str());
}

////////////////////////////////////////////////////////////////////////////////

void Command::handle(const std::vector<std::string>& params)
{
    execute(params);
}

////////////////////////////////////////////////////////////////////////////////

void StdHelp::execute(const std::vector<std::string>& params)
{
  CFinfo << m_commands << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

} // Shell
} // Tools
} // cf3





