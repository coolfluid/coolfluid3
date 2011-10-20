// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_FieldGroup_hpp
#define cf3_Mesh_FieldGroup_hpp

#include <boost/range.hpp>

#include "common/EnumT.hpp"
#include "common/Component.hpp"
#include "common/FindComponents.hpp"
#include "Mesh/LibMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CUnifiedData.hpp"
#include "Mesh/CEntities.hpp"

namespace cf3 {
namespace common {
  class CLink;
  namespace PE { class CommPattern; }
}
namespace Math { class VariablesDescriptor; }
namespace Mesh {

  class CMesh;
  class Field;
  class CRegion;
  class CElements;

  template <typename T> class CList;

////////////////////////////////////////////////////////////////////////////////

/// Component that holds Fields of the same type (topology and space)
/// @author Willem Deconinck
class Mesh_API FieldGroup : public common::Component {

public: // typedefs

  typedef boost::shared_ptr<FieldGroup> Ptr;
  typedef boost::shared_ptr<FieldGroup const> ConstPtr;

  class Mesh_API Basis
  {
  public:

    /// Enumeration of the Shapes recognized in CF
    enum Type { INVALID=-1, POINT_BASED=0,  ELEMENT_BASED=1, CELL_BASED=2, FACE_BASED=3 };

    typedef common::EnumT< Basis > ConverterBase;

    struct Mesh_API Convert : public ConverterBase
    {
      /// constructor where all the converting maps are built
      Convert();
      /// get the unique instance of the converter class
      static Convert& instance();
    };

    static std::string to_str(Type type)
    {
      return Convert::instance().to_str(type);
    }

    static Type to_enum(const std::string& type)
    {
      return Convert::instance().to_enum(type);
    }

  };

public: // functions

  /// Contructor
  /// @param name of the component
  FieldGroup ( const std::string& name );

  /// Virtual destructor
  virtual ~FieldGroup();

  /// Get the class name
  static std::string type_name () { return "FieldGroup"; }

  /// Create a new field in this group
  Field& create_field( const std::string& name, const std::string& variables_description = "scalar_same_name");

  /// Create a new field in this group
  Field& create_field( const std::string& name, Math::VariablesDescriptor& variables_descriptor);

  /// Return the topology
  CRegion& topology() const;

  /// Number of rows of contained fields
  virtual Uint size() const { return m_size; }

  /// Resize the contained fields
  void resize(const Uint size);

  /// Return the space_id
  const std::string& space() const { return m_space; }

  /// Return the space of given entities
  CSpace& space(const CEntities& entities) const { return entities.space(m_space); }

  /// Return the global index of every field row
  CList<Uint>& glb_idx() const { return *m_glb_idx; }

  /// Return the rank of every field row
  CList<Uint>& rank() const { return *m_rank; }

  /// Return the comm pattern valid for this field group. Created based on the glb_idx and rank if it didn't exist already
  common::PE::CommPattern& comm_pattern();

  /// Check if a field row is owned by this rank
  bool is_ghost(const Uint idx) const;

  /// @brief Check if all fields are compatible
  /// @throws common::InvalidStructure
  void check_sanity();

  boost::iterator_range< common::ComponentIterator<CEntities> > entities_range();
  boost::iterator_range< common::ComponentIterator<CElements> > elements_range();

  common::ComponentIteratorRange<Field> fields();

  Field& field(const std::string& name) const;

  CUnifiedData& elements_lookup() const { return *m_elements_lookup; }

  void create_connectivity_in_space();
  void bind_space();

  CTable<Uint>::ConstRow indexes_for_element(const CEntities& elements, const Uint idx) const;

  CTable<Uint>::ConstRow indexes_for_element(const Uint unified_element_idx) const;

  Field& create_coordinates();

  Field& coordinates() const;

  bool has_coordinates() const;

  Basis::Type basis() const { return m_basis; }

  void signal_create_field ( common::SignalArgs& node );

  void signature_create_field ( common::SignalArgs& node);

private: // functions

  void update();

  void config_space();

  void config_topology();

  void config_type();

  /// Triggered when the event mesh_changed
  void on_mesh_changed_event( common::SignalArgs& args );

protected:

  Basis::Type m_basis;

  std::string m_space;

  Uint m_size;

  boost::shared_ptr<common::CLink> m_topology;
  boost::shared_ptr<CList<Uint> > m_glb_idx;
  boost::shared_ptr<CList<Uint> > m_rank;
  boost::shared_ptr<CUnifiedData> m_elements_lookup;
  boost::shared_ptr<Field> m_coordinates;
  boost::weak_ptr<common::PE::CommPattern> m_comm_pattern;
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // cf3

#endif // cf3_Mesh_FieldGroup_hpp
