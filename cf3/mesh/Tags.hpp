// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_LibMesh_hpp
#error The header mesh/Tags.hpp shouldnt be included directly but rather by including LibMesh.hpp instead
#endif

#ifndef cf3_mesh_Tags_hpp
#define cf3_mesh_Tags_hpp

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////////////////

/// Class defines the tags for the mesh components
/// @author Tiago Quintino
/// @author Willem Deconinck
class Mesh_API Tags : public NonInstantiable<Tags> {
public:

  static const char * normal ();
  static const char * area ();
  static const char * volume ();

  static const char * geometry ();
  static const char * topology ();
  static const char * coordinates ();
  static const char * nodes ();
  static const char * nodes_used ();

  static const char * global_indices ();
  static const char * map_global_to_local ();

  static const char * cell_entity ();
  static const char * face_entity ();
  static const char * edge_entity ();
  static const char * point_entity();

  static const char * interface ();

  static const char * cells ();

  static const char * inner_faces ();
  static const char * outer_faces ();
  static const char * bdry_faces ();

  static const char * connectivity_table ();

  static const char * event_mesh_loaded();
  static const char * event_mesh_changed();

//  static const char * geometry_elements ();

}; // Tags

////////////////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

#endif // cf3_mesh_Tags_hpp
