// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_tools_mesh_transformer_Help_hpp
#define cf3_tools_mesh_transformer_Help_hpp

#include <boost/program_options.hpp>
#include "Tools/Shell/Interpreter.hpp"

namespace cf3 {
namespace Tools {
namespace mesh_transformer {

////////////////////////////////////////////////////////////////////////////////

class Help : public Shell::Command
{
public:
  typedef std::pair<std::string,std::string> transformers_description_t;
  
public:
  Help(Shell::commands_description& commands);
  virtual void execute( const std::vector<std::string>& params );
};

////////////////////////////////////////////////////////////////////////////////

} // mesh_transformer
} // Tools
} // cf3

#endif //cf3_tools_mesh_transformer_Help_hpp