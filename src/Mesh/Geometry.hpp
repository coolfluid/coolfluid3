// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Geometry_hpp
#define CF_Mesh_Geometry_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/Field.hpp"
#include "Mesh/FieldGroup.hpp"
#include "Mesh/LibMesh.hpp"

namespace CF {
namespace Mesh {

template <typename T> class CDynTable;

////////////////////////////////////////////////////////////////////////////////

/// Geometry component class
/// This class stores information about the nodes of the mesh
/// @author Willem Deconinck
class Mesh_API Geometry : public Mesh::FieldGroup {

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

  Field& coordinates() { return *m_coordinates; }
  const Field& coordinates() const { return *m_coordinates; }

  CDynTable<Uint>& glb_elem_connectivity() { return *m_glb_elem_connectivity; }
  const CDynTable<Uint>& glb_elem_connectivity() const { return *m_glb_elem_connectivity; }

  /// The dimension for the coordinates of the mesh
  Uint dim() const { return coordinates().row_size(); }

  virtual Uint size() const { return coordinates().size(); cf_assert(m_size == coordinates().size()); }

private: // data

  boost::shared_ptr<Field> m_coordinates;

  boost::shared_ptr<CDynTable<Uint> > m_glb_elem_connectivity;
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Geometry_hpp
