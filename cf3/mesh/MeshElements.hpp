// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_MeshElements_hpp
#define cf3_mesh_MeshElements_hpp

#include "mesh/UnifiedData.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
  
  class Mesh;

////////////////////////////////////////////////////////////////////////////////

/// This class allows to access data spread over multiple components
/// with a continuous index
/// @pre the data components must be of the same type and must have
///      a member function "Uint size() const" defined.
class Mesh_API MeshElements : public UnifiedData
{
public: //typedefs

  
  
  
public: // functions

  /// Contructor
  /// @param name of the component
  MeshElements ( const std::string& name );

  /// Virtual destructor
  virtual ~MeshElements() {}

  /// Get the class name
  static std::string type_name () { return "MeshElements"; }

  void update();
  
}; // MeshElements

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_MeshElements_hpp
