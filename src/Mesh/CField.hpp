// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CField_hpp
#define CF_Mesh_CField_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/range.hpp>
#include "Common/EnumT.hpp"

#include "Mesh/CTable.hpp"
#include "Mesh/CEntities.hpp"
namespace CF {
namespace Common
{
  class CLink;
  class PECommPattern;
}
namespace Mesh {

  class CRegion;
  template <typename T> class CList;

////////////////////////////////////////////////////////////////////////////////

/// Field component class
/// This class stores fields which can be applied
/// to fields (CField)
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API CField : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CField> Ptr;
  typedef boost::shared_ptr<CField const> ConstPtr;

  enum VarType { SCALAR=1, VECTOR_2D=2, VECTOR_3D=3, TENSOR_2D=4, TENSOR_3D=9};

  class Mesh_API Basis
  {
  public:

    /// Enumeration of the Shapes recognized in CF
    enum Type { INVALID=-1, POINT_BASED=0,  ELEMENT_BASED=1, CELL_BASED=2, FACE_BASED=3 };

    typedef Common::EnumT< Basis > ConverterBase;

    struct Mesh_API Convert : public ConverterBase
    {
      /// constructor where all the converting maps are built
      Convert();
      /// get the unique instance of the converter class
      static Convert& instance();
    };
  };

public: // functions

  /// Contructor
  /// @param name of the component
  CField ( const std::string& name );

  /// Virtual destructor
  virtual ~CField();

  /// Get the class name
  static std::string type_name () { return "CField"; }

  void create_data_storage();

  void signal_create_data_storage( Common::SignalArgs& node ) { create_data_storage(); }

  Basis::Type basis() const { return m_basis; }

  void set_basis(const Basis::Type basis) { m_basis = basis;}

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

  /// Return the table that holds the data for this field
  CTable<Real>& data() { return *m_data; }

  /// Return the const table that holds the data for this field
  const CTable<Real>& data() const { return *m_data; }

  /// Return the table that holds the data for this field
  CTable<Real>::Ptr data_ptr() { return m_data; }

  /// Return the const table that holds the data for this field
  CTable<Real>::ConstPtr data_ptr() const { return m_data; }

  void set_topology(CRegion& topology);

  const CRegion& topology() const;

  CRegion& topology();

  Uint size() const { return m_data->size(); }

  const CList<Uint>& used_nodes() const;

  const std::string& space_name() const { return m_space_name; }

  /// Operator to have modifiable access to a table-row
  /// @return A mutable row of the underlying array
  CTable<Real>::Row operator[](const Uint idx) { return m_data->array()[idx]; }

  /// Operator to have non-modifiable access to a table-row
  /// @return A const row of the underlying array
  CTable<Real>::ConstRow operator[](const Uint idx) const { return m_data->array()[idx]; }

  CTable<Real>::ConstRow coords(const Uint idx) const;

  Uint elements_start_idx(const CEntities& elements) const
  {
    cf_assert( exists_for_entities(elements) );
    return m_elements_start_idx.find(elements.as_ptr<CEntities>())->second;
  }

  bool exists_for_entities(const CEntities& elements) const
  {
    return m_elements_start_idx.find(elements.as_ptr<CEntities>()) != m_elements_start_idx.end();
  }

  boost::iterator_range< Common::ComponentIterator<CEntities const> > field_elements() const;

  Common::PECommPattern& parallelize();

  Common::PECommPattern& parallelize_with(Common::PECommPattern& comm_pattern);

  void synchronize();

private:

  Basis::Type m_basis;

  std::string m_space_name;

  Uint m_iter_stamp;
  Real m_time_stamp;

  void config_var_names();
  void config_var_types();
  void config_tree();
  void config_field_type();

  std::vector<std::string> m_var_names;
  std::vector<VarType> m_var_types;

  std::map<CEntities::ConstPtr,Uint> m_elements_start_idx;

protected:

  boost::shared_ptr<Common::CLink> m_topology;

  boost::shared_ptr<CTable<Real> > m_data;

  boost::shared_ptr<CTable<Real> > m_coords;

  boost::shared_ptr<CList<Uint> > m_used_nodes;

  boost::shared_ptr<Common::PECommPattern> m_comm_pattern;

};

////////////////////////////////////////////////////////////////////////////////
//
// class IsField2NodeBased
// {
// public:
//   IsField2NodeBased () {}
//
//   bool operator()(const CField::Ptr& component)
//   { return component->basis() == CField::POINT_BASED; }
//
//   bool operator()(const CField& component)
//   { return component.basis() == CField::NODE_BASED; }
// };
//
// class IsField2ElementBased
// {
// public:
//   IsField2ElementBased () {}
//
//   bool operator()(const CField::Ptr& component)
//   { return component->basis() == CField::ELEMENT_BASED; }
//
//   bool operator()(const CField& component)
//   { return component.basis() == CField::ELEMENT_BASED; }
// };

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CField_hpp
