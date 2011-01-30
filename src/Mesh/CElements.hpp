// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CElements_hpp
#define CF_Mesh_CElements_hpp

////////////////////////////////////////////////////////////////////////////////


#include "Mesh/CEntities.hpp"
#include "Mesh/ElementType.hpp"

namespace CF {
  namespace Common
  {
    class CLink;
  }
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// CElements component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API CElements : public CEntities {

public: // typedefs

  typedef boost::shared_ptr<CElements> Ptr;
  typedef boost::shared_ptr<CElements const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CElements ( const std::string& name );
  
  /// Initialize the CElements using the given type
  //void initialize(const std::string& element_type_name, CTable<Real>& coordinates);

  /// Initialize the CElements using the given type
  virtual void initialize(const std::string& element_type_name, CNodes& nodes);
    
  /// Virtual destructor
  virtual ~CElements();

  /// Get the class name
  static std::string type_name () { return "CElements"; }

  /// Mutable access to the connectivity table
  CTable<Uint>& connectivity_table();
  
  /// Const access to the connectivity table
  const CTable<Uint>& connectivity_table() const;

  /// Link a CFieldElements to this CElements
  void add_field_elements_link(CElements& field_elements);
  
  /// Mutable access to a field by its elements
  /// @param name of a field
  CElements& get_field_elements(const std::string& field_name);
  
  /// Const access to a field by its elements
  /// @param name of a field
  const CElements& get_field_elements(const std::string& field_name) const;
  
  /// return the number of elements
  virtual Uint size() const { return connectivity_table().size(); }

  virtual CTable<Uint>::ConstRow get_nodes(const Uint elem_idx);
  
  RealMatrix get_coordinates(const Uint elem_idx) const;

  void put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const;

protected: // data

  CTable<Uint>::Ptr m_connectivity_table;
  
  
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// between these comment lines is compatibility for the soon to be retired CField

  public:
  /// Initialize the CFieldElements using the given type
  //void initialize(const std::string& element_type_name, CTable<Real>& data);
  void initialize(CElements& elements);
    
  void add_element_based_storage();
  void add_node_based_storage(CTable<Real>& nodal_data);

  /// Mutable access to the nodal data (e.g. node coordinates);
  CTable<Real>& data();
  const CTable<Real>& data() const;

  CElements& get_geometry_elements();
  const CElements& get_geometry_elements() const;

  private:
  boost::shared_ptr<Common::CLink> m_support;
  std::string m_data_name;
  
// until here
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

};

////////////////////////////////////////////////////////////////////////////////

class IsElementsVolume
{
public:
  IsElementsVolume () {}
  
  bool operator()(const CEntities::Ptr& component)
  { return component->element_type().dimension() == component->element_type().dimensionality(); }
  
  bool operator()(const CEntities& component)
  { return component.element_type().dimension() == component.element_type().dimensionality(); }
};

class IsElementsSurface
{
public:
  IsElementsSurface () {}
  
  bool operator()(const CEntities::Ptr& component)
  { return component->element_type().dimension() == component->element_type().dimensionality() + 1; }
  
  bool operator()(const CEntities& component)
  { return component.element_type().dimension() == component.element_type().dimensionality() + 1; }
};
  
////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CElements_hpp
