// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CMesh_hpp
#define CF_Mesh_CMesh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/LibMesh.hpp"
#include "Mesh/FieldGroup.hpp"

namespace CF {
  namespace Common {
    class CLink;
  }
namespace Mesh {

  class Geometry;
  class CRegion;
  class CMeshElements;
  class MeshMetadata;

////////////////////////////////////////////////////////////////////////////////

/// Mesh component class
/// Mesh now stores:
///   - regions which subdivide in subregions
///   - arrays containing coordinates, variables, ...
/// @author Tiago Quintino
/// @author Willem Deconinck
class Mesh_API CMesh : public Common::Component {
public: // typedefs

  typedef boost::shared_ptr<CMesh> Ptr;
  typedef boost::shared_ptr<CMesh const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CMesh ( const std::string& name );

  /// Virtual destructor
  virtual ~CMesh();

  /// Get the class name
  static std::string type_name () { return "CMesh"; }

  // functions specific to the CMesh component

  /// @return the geometry topology
  CRegion& topology() const { return *m_topology; }

  void create_space( const std::string& name, const FieldGroup::Basis::Type base, const std::string& space_lib_name);
  void create_space( const std::string& name, const FieldGroup::Basis::Type base, const std::string& space_lib_name, CRegion& topology);

  FieldGroup& create_field_group( const std::string& name, const FieldGroup::Basis::Type base);
  FieldGroup& create_field_group( const std::string& name, const FieldGroup::Basis::Type base, const std::string& space);
  FieldGroup& create_field_group( const std::string& name, const FieldGroup::Basis::Type base, const std::string& space, const CRegion& topology);

  /// @brief Create new space and field-group matching the space
  /// @param [in] name            Name to be given to the space, and the field group
  /// @param [in] base            Basis of the space (POINT_BASED, CELL_BASED, FACE_BASED)
  /// @param [in] space_lib_name  Library name where all the shapefunctions can be found (e.g. CF.Mesh.LagrangeP1)
  /// @return newly created field group
  /// @note The topology this field group applies to is by default the entire mesh topology
  FieldGroup& create_space_and_field_group( const std::string& name, const FieldGroup::Basis::Type base, const std::string& space_lib_name);

  /// @brief Create new space and field-group matching the space
  /// @param [in] name            Name to be given to the space, and the field group
  /// @param [in] base            Basis of the space (POINT_BASED, CELL_BASED, FACE_BASED)
  /// @param [in] space_lib_name  Library name where all the shapefunctions can be found (e.g. CF.Mesh.LagrangeP1)
  /// @param [in] topology        The topology of the mesh this field group applies to.
  /// @return newly created field group
  FieldGroup& create_space_and_field_group( const std::string& name, const FieldGroup::Basis::Type base, const std::string& space_lib_name, CRegion& topology);

  void update_statistics();

  /// @return the nodes of the mesh
  Geometry& geometry() const;

  /// @return linearized view of all the entities in the mesh
  CMeshElements& elements() const;

  /// @return metadata component
  MeshMetadata& metadata() { return *m_metadata; }

  /// @return metadata component
  const MeshMetadata& metadata() const { return *m_metadata; }

  void signal_write_mesh ( Common::SignalArgs& node );

  void signature_write_mesh ( Common::SignalArgs& node);

  Uint dimension() const { return m_dimension; }

  Uint dimensionality() const { return m_dimensionality; }

  /// will among others set the coordinate dimension for the nodes
  void initialize_nodes(const Uint nb_nodes, const Uint dimension);

private: // data

  Uint m_dimension;

  Uint m_dimensionality;

  boost::shared_ptr<CMeshElements> m_elements;

  boost::shared_ptr<MeshMetadata> m_metadata;

  boost::shared_ptr<CRegion> m_topology;

  boost::shared_ptr<Geometry> m_nodes;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMesh_hpp
