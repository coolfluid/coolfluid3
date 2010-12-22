// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CField_hpp
#define CF_Mesh_CField_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/LibMesh.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/CTable.hpp"


namespace CF {

namespace Common 
{
  class CLink;
}

namespace Mesh {
  
  class CRegion;

////////////////////////////////////////////////////////////////////////////////

/// Field component class
/// This class stores fields which can be applied 
/// to fields (Cfield)
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API CField : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CField> Ptr;
  typedef boost::shared_ptr<CField const> ConstPtr;
  
  enum DataBasis { ELEMENT_BASED=0,  NODE_BASED=1};
	enum VarType { SCALAR=1, VECTOR_2D=2, VECTOR_3D=3, TENSOR_2D=4, TENSOR_3D=9};

public: // functions

  /// Contructor
  /// @param name of the component
  CField ( const std::string& name );

  /// Virtual destructor
  virtual ~CField();

  /// Get the class name
  static std::string type_name () { return "CField"; }

  // functions specific to the CField component
  
  /// create a Cfield component
  /// @param name of the field
  CField& synchronize_with_region(CRegion& support, const std::string& field_name = "");
	
  void create_data_storage(const DataBasis basis);

  /// create a CElements component, initialized to take connectivity data for the given type
  /// @param name of the field
  /// @param element_type_name type of the elements
  CElements& create_elements (CElements& geometry_elements);
    
  const CRegion& support() const;
  CRegion& support();
  
  /// @return the number of elements stored in this field, including any subfields
  Uint recursive_elements_count() const
  {
    Uint elem_count = 0;
    BOOST_FOREACH(const CElements& elements, Common::find_components_recursively<CElements>(*this))
    {
      elem_count += elements.elements_count();
    }
    return elem_count;
  }
  
  /// @return the number of elements stored in this field, including any subfields
  template <typename Predicate>
  Uint recursive_filtered_elements_count(const Predicate& pred) const
  {
    Uint elem_count = 0;
    BOOST_FOREACH(const CElements& elements, Common::find_components_recursively_with_filter<CElements>(*this,pred))
    {
      elem_count += elements.elements_count();
    }
    return elem_count;
  }
  
  std::string field_name() const { return m_field_name; }
  
  DataBasis basis() const { return m_basis; }
  
  void set_basis(const DataBasis& basis) { m_basis = basis;}
  
  /// @return the field with given name
  const CField& subfield(const std::string& name) const;
  
  /// @return the field with given name
  CField& subfield(const std::string& name);
  
  /// @return the elements with given name
  const CFieldElements& elements (const std::string& element_type_name) const;
  
  /// @return the elements with given name
  CFieldElements& elements (const std::string& element_type_name);
  
  std::string var_name(Uint i=0) const;
  VarType var_type(Uint i=0) const { return m_var_types[i]; }
  Uint nb_vars() const { return m_var_types.size(); }
  
  /// Find the variable index of the given variable
  Uint find_var(const std::string& vname) const;

  /// Return the start index of a given variable
  Uint var_index(const std::string& vname) const;
  
  /// Return the length (in number of Real values occupied in the data row) of the variable of the given name
  Uint var_length(const std::string& vname) const;
  
  /// Return the table that holds the data for this field
  Mesh::CTable<Real>& data_table();
  
  /// Return the const table that holds the data for this field
  const Mesh::CTable<Real>& data_table() const;
  
private:
  
  std::string m_field_name;
  
  DataBasis m_basis;
				
	void config_var_names();
	void config_var_sizes();
	void config_var_types();
	
	
	std::vector<std::string> m_var_names;
	std::vector<VarType> m_var_types;
	
	
};

////////////////////////////////////////////////////////////////////////////////
  
class IsFieldNodeBased
{
public:
  IsFieldNodeBased () {}
  
  bool operator()(const CField::Ptr& component)
  { return component->basis() == CField::NODE_BASED; }
  
  bool operator()(const CField& component)
  { return component.basis() == CField::NODE_BASED; }
};

class IsFieldElementBased
{
public:
  IsFieldElementBased () {}
  
  bool operator()(const CField::Ptr& component)
  { return component->basis() == CField::ELEMENT_BASED; }
  
  bool operator()(const CField& component)
  { return component.basis() == CField::ELEMENT_BASED; }
};
  
////////////////////////////////////////////////////////////////////////////////

  
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CField_hpp
