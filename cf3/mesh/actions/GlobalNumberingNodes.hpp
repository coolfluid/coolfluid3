// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_GlobalNumberingNodes_hpp
#define cf3_mesh_actions_GlobalNumberingNodes_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

//////////////////////////////////////////////////////////////////////////////

/// @brief Create a global number for nodes and elements of the mesh
///
/// Global numbers are created using a hash based on the coordinates
/// of the nodes, and all vertex coordinates of the elements.
/// After numbering the nodes and elements will share the global numbering
/// table given an example with 30 nodes and 30 elements on 3 processes
/// proc 1:   nodes [ 0 ->  9]
/// proc 2:   nodes [10 -> 19]
/// proc 3:   nodes [20 -> 29]
/// Through the numbering the process it belongs to can be identified:
/// - id 25 must belong to process 3
/// - id 11 must belong to process 2
/// - ...
/// @author Willem Deconinck
class mesh_actions_API GlobalNumberingNodes : public MeshTransformer
{
public: // functions

  /// constructor
  GlobalNumberingNodes( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "GlobalNumberingNodes"; }

  virtual void execute();

private: // functions

  std::size_t hash_value(const RealVector& coords);

  bool m_debug;
}; // end GlobalNumberingNodes


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_GlobalNumberingNodes_hpp
