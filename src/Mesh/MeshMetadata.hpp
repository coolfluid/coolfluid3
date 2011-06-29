// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_MeshMetadata_hpp
#define CF_Mesh_MeshMetadata_hpp

#include "Common/Component.hpp"
#include "Mesh/LibMesh.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// @brief Storage for metadata related to the mesh
///
/// This class stores metadata. This is typically used for
/// the mesh writer, that needs information about the time,
/// iteration, date, etc..
/// @author Willem Deconinck
class Mesh_API MeshMetadata : public Common::Component
{
public: //typedefs

  typedef boost::shared_ptr<MeshMetadata> Ptr;
  typedef boost::shared_ptr<MeshMetadata const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  MeshMetadata ( const std::string& name );

  /// Virtual destructor
  virtual ~MeshMetadata() {}

  /// Get the class name
  static std::string type_name () { return "MeshMetadata"; }

  boost::any& operator[](const std::string& name);
  const boost::any& operator[](const std::string& name) const;

  bool check(const std::string& name) const;

}; // MeshMetadata

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_MeshMetadata_hpp
