// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CElements_hpp
#define CF_Mesh_CElements_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/ElementType.hpp"

namespace CF {
namespace Mesh {

  class CTable;
  class CArray;
  class ElementType;
  class CFieldElements;
  template <typename T> class CList;
	
////////////////////////////////////////////////////////////////////////////////

/// CElements component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API CElements : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CElements> Ptr;
  typedef boost::shared_ptr<CElements const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CElements ( const CName& name );
  
  /// Initialize the CElements using the given type
  void initialize(const std::string& element_type_name, CArray& data);
  
  /// Virtual destructor
  virtual ~CElements();

  /// Get the class name
  static std::string type_name () { return "CElements"; }

  /// Configuration Options
  static void define_config_properties ( Common::PropertyList& options ) {}

  /// set the element type
  void set_element_type(const std::string& etype_name);

  /// return the elementType
  const ElementType& element_type() const { return *m_element_type; }
  
  /// return the number of elements
  Uint elements_count() const;
  
  /// create a CTable component and add it to the list of subcomponents
  /// @param name of the region
  CTable& create_connectivity_table ( const CName& name = "connectivity_table");

	/// create a CList<Uint> component and add it to the list of subcomponents
  /// @param name of the node list
	virtual CList<Uint>& create_node_list( const CName& name = "node_list");

	/// update the node list using the connectivity table
	CList<Uint>& update_node_list();
	
  /// Mutable access to the connectivity table
  CTable& connectivity_table();
  
  /// Const access to the connectivity table
  const CTable& connectivity_table() const;
    
  /// Mutable access to the coordinates
  virtual CArray& coordinates();
  
  /// Const access to the coordinates
  virtual const CArray& coordinates() const;
  
	/// Mutable access to the list of nodes
	CList<Uint>& node_list();
	
	/// Const access to the list of nodes
	const CList<Uint>& node_list() const;
	
	/// Link a CFieldElements to this CElements
  void add_field_elements_link(CElements& field_elements);
  
	/// Mutable access to a field by its elements
	/// @param name of a field
  CFieldElements& get_field_elements(const CName& field_name);
	
	/// Const access to a field by its elements
	/// @param name of a field
  const CFieldElements& get_field_elements(const CName& field_name) const;
    
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

protected: // data
  
  boost::shared_ptr<ElementType> m_element_type;
    
};

////////////////////////////////////////////////////////////////////////////////

class IsElementsVolume
{
public:
  IsElementsVolume () {}
  
  bool operator()(const CElements::Ptr& component)
  { return component->element_type().dimension() == component->element_type().dimensionality(); }
  
  bool operator()(const CElements& component)
  { return component.element_type().dimension() == component.element_type().dimensionality(); }
};

class IsElementsSurface
{
public:
  IsElementsSurface () {}
  
  bool operator()(const CElements::Ptr& component)
  { return component->element_type().dimension() == component->element_type().dimensionality() + 1; }
  
  bool operator()(const CElements& component)
  { return component.element_type().dimension() == component.element_type().dimensionality() + 1; }
};
  
////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CElements_hpp
