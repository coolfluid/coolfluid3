// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CEntities_hpp
#define CF_Mesh_CEntities_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/MatrixTypes.hpp"
#include "Mesh/LibMesh.hpp"
#include "Mesh/CTable.hpp"

namespace CF {
namespace Common { class CLink; }
namespace Mesh {

  template <typename T> class CList;
  class CNodes;
  class ElementType;
  class CSpace;

////////////////////////////////////////////////////////////////////////////////

/// CEntities component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API CEntities : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CEntities> Ptr;
  typedef boost::shared_ptr<CEntities const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CEntities ( const std::string& name );
  
  /// Initialize the CEntities using the given type
  void initialize(const std::string& element_type_name, CNodes& nodes);
    
  /// Virtual destructor
  virtual ~CEntities();

  /// Get the class name
  static std::string type_name () { return "CEntities"; }

  /// set the element type
  void configure_element_type();

  /// return the elementType
  const ElementType& element_type() const;

  /// Mutable access to the nodes
  CNodes& nodes();
  
  /// Const access to the coordinates
  const CNodes& nodes() const;

  /// Mutable access to the list of nodes
  CList<Uint>& glb_idx() { return *m_global_numbering; }
  
  /// Const access to the list of nodes
  const CList<Uint>& glb_idx() const { return *m_global_numbering; }
    
  /// return the number of elements
  virtual Uint size() const;

  static CList<Uint>& used_nodes(Component& parent);
  
  virtual CTable<Uint>::ConstRow get_nodes(const Uint elem_idx) const;
  
  const CSpace& space (const Uint space_idx) const;

  CSpace& create_space( const std::string& shape_function_builder_name );
  
  bool exists_space(const Uint space_idx) const;

  virtual RealMatrix get_coordinates(const Uint elem_idx) const;

  virtual void put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const;

  void allocate_coordinates(RealMatrix& coords) const;

protected: // data

  boost::shared_ptr<ElementType> m_element_type;

  boost::shared_ptr<Common::CLink> m_nodes;
  
  boost::shared_ptr<CList<Uint> > m_global_numbering;

  std::vector<boost::shared_ptr<CSpace> > m_spaces;

};

////////////////////////////////////////////////////////////////////////////////

class IsElementsVolume
{
public:
  IsElementsVolume () {}

  bool operator()(const CEntities::Ptr& component);
  bool operator()(const CEntities& component);
};

class IsElementsSurface
{
public:
  IsElementsSurface () {}

  bool operator()(const CEntities::Ptr& component);
  bool operator()(const CEntities& component);
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CEntities_hpp
