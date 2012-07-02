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

#ifndef cf3_Tools_Shell_Interpreter_hpp
#define cf3_Tools_Shell_Interpreter_hpp

#include <deque>

#include <boost/program_options.hpp>

#include "common/CF.hpp"

namespace cf3 {
namespace Tools { 
namespace Shell {

typedef boost::program_options::options_description commands_description;

////////////////////////////////////////////////////////////////////////////////

std::string default_prompt();

////////////////////////////////////////////////////////////////////////////////

/// @brief Command Line and script interpreter
///
/// Based on boost::program_options.
/// Line by line from a script is interpreted, treating
/// every line as boost::program_options arguments
/// @author Willem Deconinck
class Interpreter
{
private:
  
  typedef std::string (*prompt_function_pointer_t)();
  typedef std::string (*readline_function_pointer_t)(const std::string& line);
//  typedef void (*unrecognized_commands_handler_t)(std::vector<std::string>& unrecognized_commands);
  
  typedef boost::function< void (std::vector<std::string>&) >  unrecognized_commands_handler_t;
  
public:

  /// Constructor, taking description of commands
  Interpreter(const commands_description& desc);

  /// Constructor taking description of commands and prompt function
  Interpreter(const commands_description& desc, const prompt_function_pointer_t& prompt);

  /// splits a command_line in words
  std::vector<std::string> split_command_line(const std::string& input);

  /// function that handles a command-line
  void handle_read_line(std::string line);

  /// Start interpreting an input_stream
  void interpret(std::istream &input_stream);

  /// Start interpreting and input_stream, and use a function to modify each line
  /// before it gets handled.
  void interpret(std::istream &input_stream, readline_function_pointer_t f);
  
  /// interpret command line style arguments
  void interpret(int argc, char * argv[]);
  
  /// Output the prompt
  void write_prompt();
  
  void description(const commands_description& desc);
  
  void notify(boost::program_options::variables_map& vm);
  
  void alias(const std::string& arg);
  
  void interpret_alias(std::vector<std::string>& unrecognized_commands);

private:

  commands_description m_desc;
  const prompt_function_pointer_t m_prompt;
  std::vector<unrecognized_commands_handler_t> m_handle_unrecognized_commands;
  std::deque<std::string> m_history;
  std::map<std::string,std::string> m_alias;
};

////////////////////////////////////////////////////////////////////////////////

class Command : public commands_description
{
public:
  Command(const std::string& command, const std::string& description, commands_description& commands);
  void handle(const std::vector<std::string>& params);
  virtual void execute(const std::vector<std::string>& params) = 0;
protected:
  commands_description& m_commands;
};

////////////////////////////////////////////////////////////////////////////////

class StdHelp : public Command
{
public:
  StdHelp(const std::string& command, const std::string& description, commands_description& commands)
    : Command(command,description,commands) {}
  virtual void execute(const std::vector<std::string>& params);
};

////////////////////////////////////////////////////////////////////////////////

} // Shell
} // Tools
} // cf3

#endif // cf3_Tools_Shell_Interpreter_hpp
