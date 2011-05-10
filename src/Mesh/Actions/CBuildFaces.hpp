// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CBuildFaces_hpp
#define CF_Mesh_CBuildFaces_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshTransformer.hpp"

#include "Mesh/Actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
  class CRegion;
  class CFaceCellConnectivity;

namespace Actions {
  
//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that returns information about the mesh
/// @author Willem Deconinck
class Mesh_Actions_API CBuildFaces : public CMeshTransformer
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

  void build_face_elements(CRegion& in_region, CFaceCellConnectivity& from_face_to_cell, const bool inner);
    
  boost::shared_ptr<CFaceCellConnectivity> match_faces(CRegion& region1, CRegion& region2);
  void match_boundary(CRegion& bdry_region, CRegion& region2);

  void build_cell_face_connectivity(Component& parent);

private: // data

  bool m_store_cell2face;

}; // end CBuildFaces


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CBuildFaces_hpp
