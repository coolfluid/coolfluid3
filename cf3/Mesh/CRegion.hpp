// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_CRegion_hpp
#define cf3_Mesh_CRegion_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/FindComponents.hpp"

#include "Mesh/LibMesh.hpp"

#include "Mesh/CEntities.hpp"

namespace cf3 {
namespace Mesh {

  template <typename T> class CTable;
  class Geometry;
  class CElements;

////////////////////////////////////////////////////////////////////////////////

/// Region component class
/// This class stores
///   - subregions (same class)
///   - element sets (CElements)
/// @author Tiago Quintino, Willem Deconinck
class Mesh_API CRegion : public common::Component {

public: // typedefs

  typedef boost::shared_ptr<CRegion> Ptr;
  typedef boost::shared_ptr<CRegion const> ConstPtr;

  typedef common::ComponentIteratorRange<CEntities const> ConstElementsRange;
  typedef common::ComponentIteratorRange<CEntities>       ElementsRange;

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
  CRegion& create_region ( const std::string& name );

  /// create a CElements component, initialized to take connectivity data for the given type
  /// Set to refer to the supplied nodes
  /// @param element_type_name type of the elements
  /// @param nodes  location of the nodes the elements are linked with
  CElements& create_elements (const std::string& element_type_name, Geometry& geometry);

  /// Create a CElements with nodes unset
  CElements& create_elements (const std::string& element_type_name);

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
  Geometry& geometry() const;

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
  BOOST_FOREACH(const CEntities& elements, common::find_components_recursively_with_filter<CEntities>(*this,pred))
    elem_count += elements.size();

  return elem_count;
}

} // Mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Mesh_CRegion_hpp
