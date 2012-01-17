// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Mesh_hpp
#define cf3_mesh_Mesh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"
#include "mesh/LibMesh.hpp"
#include "mesh/Dictionary.hpp"

namespace cf3 {
  namespace common {
    class Link;
  }
namespace mesh {

  
//  class Dictionary;
  class Region;
  class MeshElements;
  class MeshMetadata;

////////////////////////////////////////////////////////////////////////////////

/// Mesh component class
/// Mesh now stores:
///   - regions which subdivide in subregions
///   - arrays containing coordinates, variables, ...
/// @author Tiago Quintino
/// @author Willem Deconinck
class Mesh_API Mesh : public common::Component {
public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  Mesh ( const std::string& name );

  /// Virtual destructor
  virtual ~Mesh();

  /// Get the class name
  static std::string type_name () { return "Mesh"; }

  // functions specific to the Mesh component

  /// @return the geometry topology
  Region& topology() const { return *m_topology; }

//  void create_space( const std::string& name, const Dictionary::Basis::Type base, const std::string& space_lib_name);
//  void create_space( const std::string& name, const Dictionary::Basis::Type base, const std::string& space_lib_name, Region& topology);

//  Dictionary& create_dict( const std::string& name, const Dictionary::Basis::Type base);
//  Dictionary& create_dict( const std::string& name, const Dictionary::Basis::Type base, const std::string& space);
//  Dictionary& create_dict( const std::string& name, const Dictionary::Basis::Type base, const std::string& space, const Region& topology);

//  /// @brief Create new space and field-group matching the space
//  /// @param [in] name            Name to be given to the space, and the field group
//  /// @param [in] base            Basis of the space (POINT_BASED, CELL_BASED, FACE_BASED)
//  /// @param [in] space_lib_name  Library name where all the shapefunctions can be found (e.g. cf3Mesh.LagrangeP1)
//  /// @return newly created field group
//  /// @note The topology this field group applies to is by default the entire mesh topology
//  Dictionary& create_space_and_dict( const std::string& name, const Dictionary::Basis::Type base, const std::string& space_lib_name);

//  /// @brief Create new space and field-group matching the space
//  /// @param [in] name            Name to be given to the space, and the field group
//  /// @param [in] base            Basis of the space (POINT_BASED, CELL_BASED, FACE_BASED)
//  /// @param [in] space_lib_name  Library name where all the shapefunctions can be found (e.g. cf3Mesh.LagrangeP1)
//  /// @param [in] topology        The topology of the mesh this field group applies to.
//  /// @return newly created field group
//  Dictionary& create_space_and_dict( const std::string& name, const Dictionary::Basis::Type base, const std::string& space_lib_name, Region& topology);

  Dictionary& create_continuous_space   ( const std::string& space_name, const std::string& space_lib_name, const std::vector< Handle<Entities> >& entities);
  Dictionary& create_continuous_space   ( const std::string& space_name, const std::string& space_lib_name, const std::vector< Handle<Region>   >& regions);
  Dictionary& create_continuous_space   ( const std::string& space_name, const std::string& space_lib_name);
  Dictionary& create_discontinuous_space( const std::string& space_name, const std::string& space_lib_name, const std::vector< Handle<Entities> >& entities);
  Dictionary& create_discontinuous_space( const std::string& space_name, const std::string& space_lib_name, const std::vector< Handle<Region>   >& regions);
  Dictionary& create_discontinuous_space( const std::string& space_name, const std::string& space_lib_name);

  void update_statistics();

  /// @return the nodes of the mesh
  Dictionary& geometry_fields() const;

  /// @return linearized view of all the entities in the mesh
  MeshElements& elements() const;

  /// @return metadata component
  MeshMetadata& metadata() { return *m_metadata; }

  /// @return metadata component
  const MeshMetadata& metadata() const { return *m_metadata; }

  void write_mesh( const common::URI& file, const std::vector<common::URI> fields = std::vector<common::URI>());

  void signal_write_mesh ( common::SignalArgs& node );

  void signature_write_mesh ( common::SignalArgs& node);

  Uint dimension() const { return m_dimension; }

  Uint dimensionality() const { return m_dimensionality; }

  /// will among others set the coordinate dimension for the nodes
  void initialize_nodes(const Uint nb_nodes, const Uint dimension);

  bool check_sanity() const;
  bool check_sanity(std::vector<std::string>& messages) const;

  void raise_mesh_loaded();

private: // data

  Uint m_dimension;

  Uint m_dimensionality;

  Handle<MeshElements> m_elements;

  Handle<MeshMetadata> m_metadata;

  Handle<Region> m_topology;

  Handle<Dictionary> m_geometry_fields;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Mesh_hpp
