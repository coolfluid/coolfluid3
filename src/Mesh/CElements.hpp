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
  namespace Common
  {
    class CLink;
    class CGroup;
  }
namespace Mesh {

  template <typename T> class CTable;
  class CFieldElements;
  template <typename T> class CList;
  class CNodes;

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
  CElements ( const std::string& name );
  
  /// Initialize the CElements using the given type
  //void initialize(const std::string& element_type_name, CTable<Real>& coordinates);

  /// Initialize the CElements using the given type
  void initialize(const std::string& element_type_name, CNodes& nodes);
    
  /// Virtual destructor
  virtual ~CElements();

  /// Get the class name
  static std::string type_name () { return "CElements"; }

  /// set the element type
  void set_element_type(const std::string& etype_name, const Uint space=0);

  /// return the elementType
  const ElementType& element_type(const Uint space=0) const;

  /// Mutable access to the connectivity table
  CTable<Uint>& connectivity_table(const Uint space=0);
  
  /// Const access to the connectivity table
  const CTable<Uint>& connectivity_table(const Uint space=0) const;
    
  /// Mutable access to the nodes
  virtual CNodes& nodes();
  
  /// Const access to the coordinates
  virtual const CNodes& nodes() const;

  /// Mutable access to the list of nodes
  CList<Uint>& glb_idx() { return *m_global_numbering; }
  
  /// Const access to the list of nodes
  const CList<Uint>& glb_idx() const { return *m_global_numbering; }
  
  /// Link a CFieldElements to this CElements
  void add_field_elements_link(CElements& field_elements);
  
  /// Mutable access to a field by its elements
  /// @param name of a field
  CFieldElements& get_field_elements(const std::string& field_name);
  
  /// Const access to a field by its elements
  /// @param name of a field
  const CFieldElements& get_field_elements(const std::string& field_name) const;
  
  /// return the number of elements
  Uint size() const { return connectivity_table().size(); }

  static CList<Uint>& used_nodes(Component& parent);

  RealMatrix element_coordinates(const Uint elem_idx, const Uint space=0) const;
  
  virtual std::vector<Uint> element_nodes(const Uint elem_idx, const Uint space=0) const;

protected: // data

  boost::shared_ptr<ElementType> m_element_type;

  Component::Ptr m_connectivity_table;

  boost::shared_ptr<Common::CLink> m_nodes;
  
  boost::shared_ptr<CList<Uint> > m_global_numbering;

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
