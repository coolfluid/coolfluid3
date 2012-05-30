// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_GlobalConnectivity_hpp
#define cf3_mesh_actions_GlobalConnectivity_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {
  
//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that finds matching nodes in given regions of the mesh
/// @author Willem Deconinck
class mesh_actions_API GlobalConnectivity : public MeshTransformer
{
public: // typedefs

    
    

public: // functions
  
  /// constructor
  GlobalConnectivity( const std::string& name );
  
  virtual ~GlobalConnectivity();
  
  /// Gets the Class name
  static std::string type_name() { return "GlobalConnectivity"; }

  virtual void execute();
  
}; // end GlobalConnectivity


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_GlobalConnectivity_hpp
