// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Tools_MeshDiff_Commands_hpp
#define cf3_Tools_MeshDiff_Commands_hpp

#include <boost/program_options.hpp>
#include "Tools/MeshDiff/LibMeshDiff.hpp"

namespace cf3 {
namespace Tools {
namespace MeshDiff {
  
////////////////////////////////////////////////////////////////////////////////

class MeshDiff_API Commands
{
public: // typedefs

  typedef boost::program_options::options_description commands_description;

public: // functions
  
  Commands();

  static commands_description description();

  static void compare(const std::vector<std::string>& params);

public: // data

  static boost::shared_ptr<common::Component> current_component;

};

////////////////////////////////////////////////////////////////////////////////

} // MeshDiff
} // Tools
} // cf3
#endif // cf3_Tools_MeshDiff_Commands_hpp
