// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Dictionary_hpp
#define cf3_mesh_Dictionary_hpp

#include <boost/cstdint.hpp>

#include "common/Map.hpp"

#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace common {
  class Link;
  template <typename T> class List;
  template <typename T> class DynTable;
  namespace PE { class CommPattern; }
}
namespace math { class VariablesDescriptor; }
namespace mesh {

  class Mesh;
  class Field;
  class Region;
  class Elements;
  class Entities;
  class Space;
  class SpaceElem;

////////////////////////////////////////////////////////////////////////////////

/// Component that holds Fields of the same type (topology and space)
/// @author Willem Deconinck
class Mesh_API Dictionary : public common::Component {

 /// @todo find workaround for following hack
friend class Mesh; // dirty (but harmless) hack, because geometry coordinates field
                   // needs to be initialized differently and added to m_fields

public: // functions

  /// Contructor
  /// @param name of the component
  Dictionary ( const std::string& name );

  /// Virtual destructor
  virtual ~Dictionary();

  /// Get the class name
  static std::string type_name () { return "Dictionary"; }

  /// Create a new field in this group
  Field& create_field( const std::string& name, const std::string& variables_description = "scalar_same_name");

  /// Create a new field in this group
  Field& create_field( const std::string& name, math::VariablesDescriptor& variables_descriptor);

  /// Number of rows of contained fields
  Uint size() const;

  /// Resize the contained fields
  void resize(const Uint size);

  /// Return the space of given entities
  const Space& space(const Entities& entities) const;

  /// Return the space of given entities
  const Handle< Space const>& space(const Handle< Entities const>& entities) const;

  /// Return the global index of every field row
  common::List<Uint>& glb_idx() { return *m_glb_idx; }

  /// Return the global index of every field row
  const common::List<Uint>& glb_idx() const { return *m_glb_idx; }

  /// Return the rank of every field row
  common::List<Uint>& rank() { return *m_rank; }

  /// Return the rank of every field row
  const common::List<Uint>& rank() const { return *m_rank; }

  /// Return a mapping between global and local indices
//  common::Map<boost::uint64_t,Uint>& glb_to_loc() { return *m_glb_to_loc; }

  /// Return a mapping between global and local indices
  const common::Map<boost::uint64_t,Uint>& glb_to_loc() const { return *m_glb_to_loc; }

  /// Node to space-element connectivity
  const common::DynTable<SpaceElem>& connectivity() const { return *m_connectivity; }

  /// Return the comm pattern valid for this field group. Created based on the glb_idx and rank if it didn't exist already
  common::PE::CommPattern& comm_pattern();

  /// Check if a field row is owned by this rank
  bool is_ghost(const Uint idx) const;

  /// @brief Check if all fields are compatible
  bool check_sanity(std::vector<std::string>& messages) const;
  bool check_sanity() const;

  const std::vector<Handle< Entities > >& entities_range() const;

  const std::vector<Handle< Space > >& spaces() const;

  Field& field(const std::string& name);

  const Field& coordinates() const;

  Field& coordinates();

  const std::vector< Handle<Field> >& fields() const { return m_fields; }

  common::DynTable<Uint>& glb_elem_connectivity();

  void signal_create_field ( common::SignalArgs& node );

  void signature_create_field ( common::SignalArgs& node);

  bool defined_for_entities(const Handle<Entities const>& entities) const;

  void add_space(const Handle<Space>& space);

  void update_structures();

  bool continuous() const { return m_is_continuous; }

  bool discontinuous() const { return !m_is_continuous; }

  void rebuild_map_glb_to_loc();

  void build();

  /// @note This is a function only for non-geometry spaces.
  virtual void rebuild_spaces_from_geometry() = 0;

  virtual void rebuild_node_to_element_connectivity() = 0;

private: // functions

  void config_space();

  void config_topology();

  void config_regions();

  void config_type();

protected: // functions

  bool has_coordinates() const;

  Field& create_coordinates();

protected:
  Handle<common::List<Uint> > m_glb_idx;
  Handle<common::List<Uint> > m_rank;
  Handle<Field> m_coordinates;
  Handle<common::DynTable<Uint> > m_glb_elem_connectivity;
  Handle<common::PE::CommPattern> m_comm_pattern;
  Handle<common::Map<boost::uint64_t,Uint> > m_glb_to_loc;
  bool m_is_continuous;

  /// Connectivity with the element of the space
  Handle<common::DynTable<SpaceElem> > m_connectivity;


private:

  std::map< Handle<Entities const> , Handle<Space const> > m_spaces_map;
  std::vector< Handle<Space   > > m_spaces;
  std::vector< Handle<Entities> > m_entities;
  std::vector< Handle<Field> > m_fields;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

#endif // cf3_mesh_Dictionary_hpp
