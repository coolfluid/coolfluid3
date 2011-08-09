// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CMesh_hpp
#define CF_Mesh_CMesh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/LibMesh.hpp"
#include "Mesh/Field.hpp"
#include "Mesh/FieldGroup.hpp"

namespace CF {
  namespace Common {
    class CLink;
  }
namespace Mesh {

  class CNodes;
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
  const CRegion& topology() const { return *m_topology; }

  /// @return the geometry topology
  CRegion& topology() { return *m_topology; }

  FieldGroup& create_field_group( const std::string& name, const FieldGroup::Basis::Type base);
  FieldGroup& create_field_group( const std::string& name, const FieldGroup::Basis::Type base, const std::string& space);
  FieldGroup& create_field_group( const std::string& name, const FieldGroup::Basis::Type base, const std::string& space, const CRegion&);

  /// Create a field
  /// @param name       Name for the field component
  /// @param base       Type of the storage method. See Field::Basis for valid values
  /// @param space      The space the field applies to (e.g. high-order shape functions)
  /// @param variables  Either a comma-separated string of the form variable_name[size], or "scalar_same_name" (the default), which indicates the field holds a single scalar
  Field& create_field( const std::string& name,
                        const Field::Basis::Type base,
                        const std::string& space = "space[0]",
                        const std::string& variables = "scalar_same_name");

  Field& create_field( const std::string& name , Field& based_on_field);

  /// Create a field
  /// @param name Name for the field component
  /// @param base Storage method
  /// @param variable_names The names of the variables to add
  /// @param variable_types The types of the variables to add
  Field& create_field( const std::string& name,
                          const Field::Basis::Type base,
                          const std::vector<std::string>& variable_names,
                          const std::vector<Field::VarType> variable_types);


  /// Create a field containing a single scalar
  Field& create_scalar_field( const std::string& field_name, const std::string& variable_name, const Field::Basis::Type base);

  Field& create_scalar_field( const std::string& name , Field& based_on_field);

  void update_statistics();

  /// @return the nodes of the mesh , modifiable access
  CNodes& nodes();

  /// @return the nodes of the mesh , non-modifiable access
  const CNodes& nodes() const;

  /// @return linearized view of all the entities in the mesh
  CMeshElements& elements();

  /// @return linearized view of all the entities in the mesh
  const CMeshElements& elements() const;
\
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

  boost::shared_ptr<CNodes> m_nodes;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMesh_hpp
