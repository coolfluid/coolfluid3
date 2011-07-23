// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LibMesh_hpp
#error The header Mesh/Tags.hpp shouldnt be included directly but rather by including LibMesh.hpp instead
#endif

#ifndef CF_Mesh_Tags_hpp
#define CF_Mesh_Tags_hpp

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////////////////

/// Class defines the tags for the mesh components
/// @author Tiago Quintino
/// @author Willem Deconinck
class Mesh_API Tags : public NonInstantiable<Tags> {
public:

  static const char * normal ();
  static const char * area ();
  static const char * volume ();

  static const char * coordinates ();
  static const char * nodes ();
  static const char * nodes_used ();

  static const char * global_elem_indices ();
  static const char * global_node_indices ();

  static const char * cell_entity ();
  static const char * face_entity ();
  static const char * edge_entity ();
  static const char * point_entity();

  static const char * interface ();

  static const char * inner_faces ();
  static const char * outer_faces ();

  static const char * connectivity_table ();

  static const char * geometry_elements ();

}; // Tags

////////////////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

#endif // CF_Mesh_Tags_hpp
