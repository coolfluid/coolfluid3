// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CField2_hpp
#define CF_Mesh_CField2_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Log.hpp"

#include "Mesh/LibMesh.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"


namespace CF {

namespace Common 
{
  class CLink;
}

namespace Mesh {
  
  class CRegion;
  class CNodes;
  class CFieldView;
 
////////////////////////////////////////////////////////////////////////////////

/// Field component class
/// This class stores fields which can be applied 
/// to fields (CField2)
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API CField2 : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CField2> Ptr;
  typedef boost::shared_ptr<CField2 const> ConstPtr;
  
  typedef CTable<Real>::ArrayT::array_view<2>::type View;
  typedef const CTable<Real>::ArrayT::const_array_view<2>::type ConstView;
  
  enum DataBasis { ELEMENT_BASED=0,  NODE_BASED=1};
  enum VarType { SCALAR=1, VECTOR_2D=2, VECTOR_3D=3, TENSOR_2D=4, TENSOR_3D=9};

public: // functions

  /// Contructor
  /// @param name of the component
  CField2 ( const std::string& name );

  /// Virtual destructor
  virtual ~CField2();

  /// Get the class name
  static std::string type_name () { return "CField2"; }
  
  void create_data_storage();
    
  DataBasis basis() const { return m_basis; }
  
  void set_basis(const DataBasis& basis) { m_basis = basis;}
    
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
  
  const CRegion& topology() const;

  CRegion& topology();
  
  Uint size() const { return m_data->size(); }
  
  const CList<Uint>& used_nodes() const;
  
  /// Operator to have modifiable access to a table-row
  /// @return A mutable row of the underlying array
  CTable<Real>::Row operator[](const Uint idx) { return m_data->array()[idx]; }

  /// Operator to have non-modifiable access to a table-row
  /// @return A const row of the underlying array
  CTable<Real>::ConstRow operator[](const Uint idx) const { return m_data->array()[idx]; }
  
  CTable<Real>::ConstRow coords(const Uint idx) const;
  
  const std::string& registration_name() const { return m_registration_name; }
  
  Uint elements_start_idx(const CElements& elements)
  {
    return m_elements_start_idx[&elements];
  }
  
private:
  
  std::string m_registration_name;
  
  DataBasis m_basis;

  void config_var_names();
  void config_var_sizes();
  void config_var_types();
  void config_tree();
  void config_field_type();
  
  std::vector<std::string> m_var_names;
  std::vector<VarType> m_var_types;
  
  std::map<CElements const*,Uint> m_elements_start_idx;
  
protected: 

  boost::shared_ptr<Common::CLink> m_topology;

  boost::shared_ptr<CTable<Real> > m_data;

  boost::shared_ptr<CTable<Real> > m_coords;
  
  boost::shared_ptr<CList<Uint> > m_used_nodes;
  
};

////////////////////////////////////////////////////////////////////////////////
  
class IsField2NodeBased
{
public:
  IsField2NodeBased () {}
  
  bool operator()(const CField2::Ptr& component)
  { return component->basis() == CField2::NODE_BASED; }
  
  bool operator()(const CField2& component)
  { return component.basis() == CField2::NODE_BASED; }
};

class IsField2ElementBased
{
public:
  IsField2ElementBased () {}
  
  bool operator()(const CField2::Ptr& component)
  { return component->basis() == CField2::ELEMENT_BASED; }
  
  bool operator()(const CField2& component)
  { return component.basis() == CField2::ELEMENT_BASED; }
};
  
////////////////////////////////////////////////////////////////////////////////

class Mesh_API CFieldView : public Common::Component
{
public: // typedefs

  typedef boost::shared_ptr<CFieldView> Ptr;
  typedef boost::shared_ptr<CFieldView const> ConstPtr;

  typedef CTable<Real>::Row View;
  typedef CTable<Real>::ConstRow ConstView;
  
protected: // typedefs

  typedef boost::multi_array_types::index_range Range;

public: // functions

  /// Contructor
  /// @param name of the component
  CFieldView ( const std::string& name ) : Common::Component (name)
  {
    m_size=0;
    m_start_idx=0;
    m_end_idx=0;
    m_stride=0;
  }

  /// Virtual destructor
  virtual ~CFieldView() {}

  /// Get the class name
  static std::string type_name () { return "CFieldView"; }

  /// @return end_idx
  Uint initialize(CField2& field, CElements::Ptr elements = CElements::Ptr());
  
  CField2::View operator()()
  {
    return m_field_data.lock()->array()[ boost::indices[Range(m_start_idx,m_end_idx)][Range()] ];
  }

  CField2::ConstView operator()() const
  {
    return m_field_data.lock()->array()[ boost::indices[Range(m_start_idx,m_end_idx)][Range()] ];
  }
  
  CTable<Real>::Row operator[](const Uint idx)
  {
    cf_assert( idx < m_size );
    Uint data_idx = m_start_idx+idx;
    return m_field_data.lock()->array()[data_idx];
  }

  CTable<Real>::ConstRow operator[](const Uint idx) const
  {
    cf_assert( idx < m_size );
    Uint data_idx = m_start_idx+idx;
    return m_field_data.lock()->array()[data_idx];
  }

  CField2& field() { return *m_field.lock(); }
  
  CElements& elements() { return *m_elements.lock(); }
  
  Uint stride() const { return m_stride; }
  
  Uint size() const { return m_size; }
  
  template <typename T>
  T& as() { return *as_ptr<T>(); }

  template <typename T>
  boost::shared_ptr<T> as_ptr() { return boost::static_pointer_cast<T>(self()); }
  
  void set_elements(CElements& elements) 
  { 
    cf_assert( is_not_null(m_field.lock()) );
    m_elements = elements.as_type<CElements>();
    m_stride = 1; // this is the number of states per element (high order methods)
    m_start_idx = m_field.lock()->elements_start_idx(elements);
    m_end_idx = m_start_idx + m_stride * elements.size();
    m_size = m_end_idx - m_start_idx;
  }
  
  void set_elements(CElements::Ptr elements)
  {
    cf_assert( is_not_null(m_field.lock()) );
    m_elements = elements; 
    m_stride = 1; // this is the number of states per element (high order methods)
    m_start_idx = m_field.lock()->elements_start_idx(*elements);
    m_end_idx = m_start_idx + m_stride * elements->size();
    m_size = m_end_idx - m_start_idx;
  }

  void set_field(CField2& field) 
  { 
    m_field = field.as_type<CField2>(); 
    m_field_data = field.data().as_type<CTable<Real> >();
    cf_assert( is_not_null(m_field_data.lock()) );
  }
  
  void set_field(CField2::Ptr field) 
  { 
    m_field = field;
    m_field_data = field->data().as_type<CTable<Real> >();
    cf_assert( is_not_null(m_field_data.lock()) );
  }
  
protected: 

  Uint m_start_idx;
  Uint m_end_idx;
  Uint m_stride;
  Uint m_size;

  boost::weak_ptr<CField2>       m_field;
  boost::weak_ptr<CTable<Real> > m_field_data;
  boost::weak_ptr<CElements>     m_elements;

};

////////////////////////////////////////////////////////////////////////////////

class Mesh_API CMultiStateFieldView : public CFieldView
{
public: // typedefs

  typedef boost::shared_ptr<CMultiStateFieldView> Ptr;
  typedef boost::shared_ptr<CMultiStateFieldView const> ConstPtr;

  typedef CTable<Real>::ArrayT::array_view<2>::type View;
  typedef const CTable<Real>::ArrayT::const_array_view<2>::type ConstView;

public: // functions

  /// Contructor
  /// @param name of the component
  CMultiStateFieldView ( const std::string& name ) : CFieldView (name) {}

  /// Virtual destructor
  virtual ~CMultiStateFieldView() {}

  /// Get the class name
  static std::string type_name () { return "CMultiStateFieldView"; }
  
  View operator[](const Uint idx)
  {
    cf_assert( idx < m_size );    
    Uint data_idx = m_start_idx+idx;
    Range range = Range().start(data_idx).finish(data_idx + m_stride);
    cf_assert( is_not_null(m_field_data.lock()) );
    return m_field_data.lock()->array()[ boost::indices[range][Range()] ];
  }

  ConstView operator[](const Uint idx) const
  {
    cf_assert( idx < m_size );    
    Uint data_idx = m_start_idx+idx;
    Range range = Range().start(data_idx).finish(data_idx + m_stride);
    cf_assert( is_not_null(m_field_data.lock()) );
    return m_field_data.lock()->array()[ boost::indices[range][Range()] ];
  }
  
};

////////////////////////////////////////////////////////////////////////////////

class Mesh_API CScalarFieldView : public CFieldView
{
public: // typedefs

  typedef boost::shared_ptr<CScalarFieldView> Ptr;
  typedef boost::shared_ptr<CScalarFieldView const> ConstPtr;

  typedef Real& View;
  typedef const Real& ConstView;

public: // functions

  /// Contructor
  /// @param name of the component
  CScalarFieldView ( const std::string& name ) : CFieldView (name) {}

  /// Virtual destructor
  virtual ~CScalarFieldView() {}

  /// Get the class name
  static std::string type_name () { return "CScalarFieldView"; }

  Real& operator[](const Uint idx)
  {
    cf_assert( idx < m_size );    
    Uint data_idx = m_start_idx+idx;
    cf_assert( is_not_null(m_field_data.lock()) );
    return m_field_data.lock()->array()[data_idx][0];
  }

  const Real& operator[](const Uint idx) const
  {
    cf_assert( idx < m_size );    
    Uint data_idx = m_start_idx+idx;
    cf_assert( is_not_null(m_field_data.lock()) );
    return m_field_data.lock()->array()[data_idx][0];
  }

};

  
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CField2_hpp
