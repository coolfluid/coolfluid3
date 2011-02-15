// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CField2_hpp
#define CF_Mesh_CField2_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/EnumT.hpp"

#include "Mesh/CTable.hpp"

namespace CF {
namespace Common 
{
  class CLink;
}
namespace Mesh {
  
  class CRegion;
  class CEntities;
  template <typename T> class CList;
  
////////////////////////////////////////////////////////////////////////////////

/// Field component class
/// This class stores fields which can be applied 
/// to fields (CField2)
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API CField2 : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CField2> Ptr;
  typedef boost::shared_ptr<CField2 const> ConstPtr;
  
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
  CField2 ( const std::string& name );

  /// Virtual destructor
  virtual ~CField2();

  /// Get the class name
  static std::string type_name () { return "CField2"; }
  
  void create_data_storage();
    
  Basis::Type basis() const { return m_basis; }
  
  void set_basis(const Basis::Type basis) { m_basis = basis;}
    
  std::string var_name(Uint i=0) const;
  
  VarType var_type(Uint i=0) const { return m_var_types[i]; }
  
  Uint nb_vars() const { return m_var_types.size(); }
  
  /// Find the variable index of the given variable
  Uint var_number(const std::string& vname) const;

  /// Return the start index of a given variable
  Uint var_index(const std::string& vname) const;
  
  /// Return the length (in number of Real values occupied in the data row) of the variable of the given name
  VarType var_type(const std::string& vname) const;
  
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
  
  Uint space_idx() const { return m_space_idx; }
  
  /// Operator to have modifiable access to a table-row
  /// @return A mutable row of the underlying array
  CTable<Real>::Row operator[](const Uint idx) { return m_data->array()[idx]; }

  /// Operator to have non-modifiable access to a table-row
  /// @return A const row of the underlying array
  CTable<Real>::ConstRow operator[](const Uint idx) const { return m_data->array()[idx]; }
  
  CTable<Real>::ConstRow coords(const Uint idx) const;
  
  const std::string& registration_name() const { return m_registration_name; }
  
  Uint elements_start_idx(const CEntities& elements) const
  {
    return m_elements_start_idx.find(&elements)->second;
  }
  
  bool exists_for_entities(const CEntities& elements) const
  {
    return m_elements_start_idx.find(&elements) != m_elements_start_idx.end();
  }
  
private:
  
  std::string m_registration_name;
  
  Basis::Type m_basis;
  
  Uint m_space_idx;

  void config_var_names();
  void config_var_types();
  void config_tree();
  void config_field_type();
  
  std::vector<std::string> m_var_names;
  std::vector<VarType> m_var_types;
  
  std::map<CEntities const*,Uint> m_elements_start_idx;
  
protected: 

  boost::shared_ptr<Common::CLink> m_topology;

  boost::shared_ptr<CTable<Real> > m_data;

  boost::shared_ptr<CTable<Real> > m_coords;
  
  boost::shared_ptr<CList<Uint> > m_used_nodes;
  
};

////////////////////////////////////////////////////////////////////////////////
//   
// class IsField2NodeBased
// {
// public:
//   IsField2NodeBased () {}
//   
//   bool operator()(const CField2::Ptr& component)
//   { return component->basis() == CField2::POINT_BASED; }
//   
//   bool operator()(const CField2& component)
//   { return component.basis() == CField2::NODE_BASED; }
// };
// 
// class IsField2ElementBased
// {
// public:
//   IsField2ElementBased () {}
//   
//   bool operator()(const CField2::Ptr& component)
//   { return component->basis() == CField2::ELEMENT_BASED; }
//   
//   bool operator()(const CField2& component)
//   { return component.basis() == CField2::ELEMENT_BASED; }
// };

////////////////////////////////////////////////////////////////////////////////
  
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CField2_hpp
