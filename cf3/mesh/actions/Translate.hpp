// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_Translate_hpp
#define cf3_mesh_actions_Translate_hpp

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
/// that Translates the mesh with a given translation vector
/// @author Willem Deconinck
class mesh_actions_API Translate : public MeshTransformer
{   
public: // functions
  
  /// constructor
  Translate( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "Translate"; }

  virtual void execute();
  
}; // end Translate

////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_Translate_hpp
