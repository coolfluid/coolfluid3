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

#include "Common/Exception.hpp"
#include "Common/Log.hpp"
#include "Tools/CommandLineInterpreter/CommandLineInterpreter.hpp"
#include <iostream>
#include <boost/bind.hpp>

using namespace CF::Common;

namespace CF {
namespace Tools { 
namespace CommandLineInterpreter {

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
    boost::program_options::variables_map vm;
    boost::program_options::store(
      boost::program_options::command_line_parser(args).options(*m_desc).run(), 
      vm);
    boost::program_options::notify(vm);
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
  char *line = NULL;

  while (line = boost::bind(f, m_prompt().c_str())())
  {
    if (!line)
      continue ;
      
    std::string command = line;
    free(line);
    handle_read_line(command);
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
