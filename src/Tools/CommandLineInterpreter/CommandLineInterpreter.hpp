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

#ifndef CF_Tools_CommandLineInterpreter_CommandLineInterpreter_hpp
#define CF_Tools_CommandLineInterpreter_CommandLineInterpreter_hpp

#include <deque>
#include "Common/CF.hpp"
#include <boost/program_options.hpp>

namespace CF {
namespace Tools { 
namespace CommandLineInterpreter {

////////////////////////////////////////////////////////////////////////////////

std::string default_prompt();
  
////////////////////////////////////////////////////////////////////////////////

class CommandLineInterpreter
{
private:
  
  typedef std::string (*prompt_function_pointer_t)();
  typedef std::string (*readline_function_pointer_t)(const std::string& line);
  typedef boost::program_options::options_description commands_description;
  
public:

  /// Constructor, taking description of commands
  CommandLineInterpreter(const commands_description& desc);

  /// Constructor taking description of commands and prompt function
  CommandLineInterpreter(const commands_description& desc, const prompt_function_pointer_t& prompt);

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
  
  void set_description(const commands_description& desc);
  
  void notify(boost::program_options::variables_map& vm);
private:

  commands_description m_desc;
  const prompt_function_pointer_t m_prompt;
  std::deque<std::string> m_history;
};

////////////////////////////////////////////////////////////////////////////////

} // CommandLineInterpreter
} // Tools
} // CF

#endif // CF_Tools_CommandLineInterpreter_CommandLineInterpreter_hpp
