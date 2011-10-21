// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_CBuildFaces_hpp
#define cf3_mesh_actions_CBuildFaces_hpp

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
class mesh_actions_API CBuildFaces : public MeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CBuildFaces> Ptr;
    typedef boost::shared_ptr<CBuildFaces const> ConstPtr;

public: // functions
  
  /// constructor
  CBuildFaces( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CBuildFaces"; }

  virtual void execute();
  
  /// brief description, typically one line
  virtual std::string brief_description() const;
  
  /// extended help that user can query
  virtual std::string help() const;
  
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

}; // end CBuildFaces


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CBuildFaces_hpp
