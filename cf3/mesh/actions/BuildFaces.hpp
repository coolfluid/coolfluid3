// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_BuildFaces_hpp
#define cf3_mesh_actions_BuildFaces_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshTransformer.hpp"

#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  class Region;
  class FaceCellConnectivity;

namespace actions {

//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that returns information about the mesh
/// @author Willem Deconinck
class mesh_actions_API BuildFaces : public MeshTransformer
{
public: // functions

  /// constructor
  BuildFaces( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "BuildFaces"; }

  virtual void execute();

private: // functions

  void make_interfaces(Component& parent);

  void build_face_cell_connectivity_bottom_up(Component& parent);
  void build_faces_bottom_up(Component& parent);

  void build_face_elements(Region& in_region, FaceCellConnectivity& from_face_to_cell, const bool inner);

  boost::shared_ptr<FaceCellConnectivity> match_faces(Region& region1, Region& region2);
  void match_boundary(Region& bdry_region, Region& region2);

  void build_cell_face_connectivity(Component& parent);

private: // data

  bool m_store_cell2face;

}; // end BuildFaces


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_BuildFaces_hpp
