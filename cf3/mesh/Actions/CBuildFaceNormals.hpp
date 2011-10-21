// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_CBuildFaceNormals_hpp
#define cf3_mesh_CBuildFaceNormals_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/CMeshTransformer.hpp"

#include "mesh/Actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace Actions {
  
//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that returns information about the mesh
/// @author Willem Deconinck
class Mesh_Actions_API CBuildFaceNormals : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CBuildFaceNormals> Ptr;
    typedef boost::shared_ptr<CBuildFaceNormals const> ConstPtr;

public: // functions
  
  /// constructor
  CBuildFaceNormals( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CBuildFaceNormals"; }

  virtual void execute();
  
  /// brief description, typically one line
  virtual std::string brief_description() const;
  
  /// extended help that user can query
  virtual std::string help() const;
  
  enum {FIRST=0};
}; // end CBuildFaceNormals


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CBuildFaceNormals_hpp
