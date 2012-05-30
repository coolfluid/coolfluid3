// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_Info_hpp
#define cf3_mesh_actions_Info_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshTransformer.hpp"
#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that returns information about the mesh
/// @author Willem Deconinck
class mesh_actions_API Info : public MeshTransformer
{
public: // functions
  
  /// constructor
  Info( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "Info"; }

  virtual void execute();

private: // functions
 
  std::string print_region_tree(const Region& region, Uint level=0);
  std::string print_elements(const Component& region, Uint level=0);
  
}; // end Info


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_Info_hpp
