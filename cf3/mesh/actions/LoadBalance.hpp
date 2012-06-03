// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_LoadBalance_hpp
#define cf3_mesh_actions_LoadBalance_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

//////////////////////////////////////////////////////////////////////////////

/// @brief Load Balance the mesh
///
/// @post After this, the mesh is ready to be parallellized
/// @author Willem Deconinck
class mesh_actions_API LoadBalance : public MeshTransformer
{
public: // functions

  /// constructor
  LoadBalance( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "LoadBalance"; }

  virtual void execute();

private:

  Handle<MeshTransformer> m_partitioner;

}; // end LoadBalance


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_LoadBalance_hpp
