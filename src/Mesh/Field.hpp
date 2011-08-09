// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Field_hpp
#define CF_Mesh_Field_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/FieldGroup.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CEntities.hpp"
#include "Mesh/CElements.hpp"

namespace CF {
namespace Common { class CLink; class CommPattern; }

namespace Mesh {

  class CRegion;

////////////////////////////////////////////////////////////////////////////////

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

  Uint nb_vars() const { return m_var_types.size(); }

  /// True if the field contains a variable with the given name
  bool has_variable(const std::string& vname) const;

  /// Find the variable index of the given variable
  Uint var_number(const std::string& vname) const;

  /// Return the start index of a given variable
  Uint var_index(const std::string& vname) const;

  /// Return the start index of a given variable number
  Uint var_index(const Uint var_nb) const;

  /// Return the length (in number of Real values occupied in the data row) of the variable of the given name
  VarType var_type(const std::string& vname) const;

  /// Return the length (in number of Real values occupied in the data row) of the variable of the given var number
  VarType var_type(const Uint i=0) const { return m_var_types[i]; }

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

  boost::iterator_range< Common::ComponentIterator<CEntities> > entities_range() { return field_group().entities_range(); }

  boost::iterator_range< Common::ComponentIterator<CElements> > elements_range() { return field_group().elements_range(); }

  Field& coordinates() const { return field_group().coordinates(); }

private:

  void config_var_names();
  void config_var_types();

  std::vector<std::string> m_var_names;
  std::vector<VarType> m_var_types;

  FieldGroup::Basis::Type m_basis;
  boost::weak_ptr<CRegion> m_topology;
  boost::weak_ptr<FieldGroup> m_field_group;


  boost::weak_ptr<Common::CommPattern> m_comm_pattern;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Field_hpp
