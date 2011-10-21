// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Field_hpp
#define cf3_mesh_Field_hpp

#include "mesh/FieldGroup.hpp"
#include "mesh/CTable.hpp"
#include "mesh/CEntities.hpp"
#include "mesh/CElements.hpp"

namespace cf3 {

namespace common
{
  class Link;
  namespace PE { class CommPattern; }
}
namespace math { class VariablesDescriptor; }


namespace mesh {

  class CRegion;

////////////////////////////////////////////////////////////////////////////////////////////

/// Field component class
/// This class stores fields which can be applied
/// to fields (Field)
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API Field : public CTable<Real> {

public: // typedefs

  typedef boost::shared_ptr<Field> Ptr;
  typedef boost::shared_ptr<Field const> ConstPtr;

  enum VarType { SCALAR=1, VECTOR_2D=2, VECTOR_3D=3, TENSOR_2D=4, TENSOR_3D=9};

public: // functions

  /// Contructor
  /// @param name of the component
  Field ( const std::string& name );

  /// Virtual destructor
  virtual ~Field();

  /// Get the class name
  static std::string type_name () { return "Field"; }

  FieldGroup::Basis::Type basis() const { return m_basis; }

  void set_basis(const FieldGroup::Basis::Type basis) { m_basis = basis;}

  std::string var_name(Uint i=0) const;

  Uint nb_vars() const;

  /// True if the field contains a variable with the given name
  bool has_variable(const std::string& vname) const;

  /// Find the variable index of the given variable
  Uint var_number(const std::string& vname) const;

  /// Return the start index of a given variable
  Uint var_index(const std::string& vname) const;

  /// Return the start index of a given variable number
  Uint var_index(const Uint var_nb) const;

  /// Return the length (in number of Real values occupied in the data row) of the variable of the given name
  VarType var_length(const std::string& vname) const;

  /// Return the length (in number of Real values occupied in the data row) of the variable of the given var number
  VarType var_length(const Uint i=0) const;

  void set_topology(CRegion& topology);

  CRegion& topology() const;

  void set_field_group(FieldGroup& field_group);

  FieldGroup& field_group() const;

  virtual void resize(const Uint size);

  CTable<Uint>::ConstRow indexes_for_element(const CEntities& elements, const Uint idx) const;

  CTable<Uint>::ConstRow indexes_for_element(const Uint unified_element_idx) const;

  CList<Uint>& glb_idx() const { return field_group().glb_idx(); }

  CList<Uint>& rank() const { return field_group().rank(); }

  bool is_ghost(const Uint idx) const { return field_group().is_ghost(idx); }

  const std::string& space() const { return field_group().space(); }

  CSpace& space(const CEntities& entities) const { return entities.space(field_group().space()); }

  boost::iterator_range< common::ComponentIterator<CEntities> > entities_range() { return field_group().entities_range(); }

  boost::iterator_range< common::ComponentIterator<CElements> > elements_range() { return field_group().elements_range(); }

  Field& coordinates() const { return field_group().coordinates(); }

  common::PE::CommPattern& parallelize_with( common::PE::CommPattern& comm_pattern );

  common::PE::CommPattern& parallelize();

  void synchronize();

  CUnifiedData& elements_lookup() const { return field_group().elements_lookup(); }

  math::VariablesDescriptor& descriptor() const { return *m_descriptor.lock(); }

  void set_descriptor(math::VariablesDescriptor& descriptor);

  void create_descriptor(const std::string& description, const Uint dimension=0);

private:

  void config_var_names();
  void config_var_types();

  FieldGroup::Basis::Type m_basis;
  boost::weak_ptr<CRegion> m_topology;
  boost::weak_ptr<FieldGroup> m_field_group;

  boost::weak_ptr< common::PE::CommPattern > m_comm_pattern;

  boost::weak_ptr< math::VariablesDescriptor > m_descriptor;
};

////////////////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

#endif // cf3_mesh_Field_hpp
