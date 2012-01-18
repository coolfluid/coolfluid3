// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_CreateSpaceP0_hpp
#define cf3_mesh_actions_CreateSpaceP0_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshTransformer.hpp"

#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {
  
//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that creates a P0 space for every element in the mesh
/// @author Willem Deconinck
class mesh_actions_API CreateSpaceP0 : public MeshTransformer
{
public: // typedefs

    
    

public: // functions
  
  /// constructor
  CreateSpaceP0( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CreateSpaceP0"; }

  virtual void execute();

}; // end CreateSpaceP0


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_CreateSpaceP0_hpp
