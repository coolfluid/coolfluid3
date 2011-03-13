// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CRegion_hpp
#define CF_Mesh_CRegion_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/FindComponents.hpp"

#include "Mesh/LibMesh.hpp"

#include "Mesh/CEntities.hpp"

namespace CF {
namespace Mesh {
  
  template <typename T> class CTable;
  class CNodes;
  class CElements;
  
////////////////////////////////////////////////////////////////////////////////

/// Region component class
/// This class stores
///   - subregions (same class)
///   - element sets (CElements)
/// @author Tiago Quintino, Willem Deconinck
class Mesh_API CRegion : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CRegion> Ptr;
  typedef boost::shared_ptr<CRegion const> ConstPtr;

  typedef Common::DerivedComponentIteratorRange<CEntities const> ConstElementsRange;
  typedef Common::DerivedComponentIteratorRange<CEntities>       ElementsRange;

public: // functions

  /// Contructor
  /// @param name of the component
  CRegion ( const std::string& name );

  /// Virtual destructor
  virtual ~CRegion();

  /// Get the class name
  static std::string type_name () { return "CRegion"; }

  // functions specific to the CRegion component

  /// create a CRegion component
  /// @param name of the region
  CRegion& create_region ( const std::string& name, bool ensure_unique = false );
  
  /// create a CElements component, initialized to take connectivity data for the given type
  /// @param name of the region
  /// @param element_type_name type of the elements
  CElements& create_elements (const std::string& element_type_name, CNodes& nodes);
  
  /// create a nodes component, initialized with the coordinate dimension
  /// @param dim dimension of the node coordinates
  CNodes& create_nodes(const Uint& dim);
  
  /// @return the number of elements stored in this region, including any subregions
  Uint recursive_elements_count() const;

  /// @return the number of elements stored in this region, including any subregions
  template <typename Predicate>
    Uint recursive_filtered_elements_count(const Predicate& pred) const;

  Uint recursive_nodes_count();

  /// @return the subregion with given name
  const CRegion& subregion(const std::string& name) const;
  
  /// @return the subregion with given name
  CRegion& subregion(const std::string& name);
  
  /// @return the elements with given name
  const CElements& elements (const std::string& element_type_name) const;
  
  /// @return the elements with given name
  CElements& elements (const std::string& element_type_name);

  /// @return nodes of the mesh
  CNodes& nodes();
  
  /// @return nodes of the mesh
  const CNodes& nodes() const;
  
  /// @return non-modifiable range of elements that are searched for recursively
  /// for use with boost_foreach(const CElements& elements, region.elements_range() )
  ConstElementsRange elements_range() const;

  /// @return modifiable range of elements that are searched for recursively
  /// for use with boost_foreach(CElements& elements, region.elements_range() )  
  ElementsRange elements_range();
  
private: // data

}; // CRegion

////////////////////////////////////////////////////////////////////////////////

template <typename Predicate>
inline Uint CRegion::recursive_filtered_elements_count(const Predicate& pred) const
{
  Uint elem_count = 0;
  BOOST_FOREACH(const CEntities& elements, Common::find_components_recursively_with_filter<CEntities>(*this,pred))
    elem_count += elements.size();

  return elem_count;
}

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CRegion_hpp
