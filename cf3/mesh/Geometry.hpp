// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Geometry_hpp
#define cf3_mesh_Geometry_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/Field.hpp"
#include "mesh/FieldGroup.hpp"
#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace common { template <typename T> class DynTable; }
namespace mesh {


////////////////////////////////////////////////////////////////////////////////

/// Geometry component class
/// This class stores information about the nodes of the mesh
/// @author Willem Deconinck
class Mesh_API Geometry : public mesh::FieldGroup {

public: // typedefs

  typedef boost::shared_ptr<Geometry> Ptr;
  typedef boost::shared_ptr<Geometry const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  Geometry ( const std::string& name );

  /// Virtual destructor
  virtual ~Geometry();

  /// Get the class name
  static std::string type_name () { return "Geometry"; }

  Field& coordinates() const { return *m_coordinates; }

  common::DynTable<Uint>& glb_elem_connectivity() const { return *m_glb_elem_connectivity; }

  /// The dimension for the coordinates of the mesh
  Uint dim() const { return coordinates().row_size(); }

  virtual Uint size() const { return coordinates().size(); cf3_assert(m_size == coordinates().size()); }

private: // data

  boost::shared_ptr<common::DynTable<Uint> > m_glb_elem_connectivity;
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Geometry_hpp
