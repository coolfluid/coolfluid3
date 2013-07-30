// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_CF3Mesh_Writer_hpp
#define cf3_mesh_CF3Mesh_Writer_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshWriter.hpp"
#include "mesh/GeoShape.hpp"

#include "mesh/cf3mesh/LibCF3Mesh.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace cf3mesh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines CF3Mesh mesh format writer
/// @author Bart Janssens
class CF3Mesh_API Writer : public MeshWriter
{
public: // functions

  /// constructor
  Writer( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "Writer"; }

  virtual void write();

  virtual std::string get_format() { return "CF3Mesh"; }

  virtual std::vector<std::string> get_extensions();
}; // end Writer


////////////////////////////////////////////////////////////////////////////////

} // cf3mesh
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CF3Mesh_Writer_hpp
