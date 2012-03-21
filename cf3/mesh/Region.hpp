// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Region_hpp
#define cf3_mesh_Region_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/FindComponents.hpp"

#include "mesh/LibMesh.hpp"

#include "mesh/Entities.hpp"

namespace cf3 {
namespace mesh {

  class Dictionary;
  

  class Elements;

////////////////////////////////////////////////////////////////////////////////

/// Region component class
/// This class stores
///   - subregions (same class)
///   - element sets (Elements)
/// @author Tiago Quintino, Willem Deconinck
class Mesh_API Region : public common::Component {

public: // typedefs

  
  

  typedef common::ComponentIteratorRange<Entities const> ConstElementsRange;
  typedef common::ComponentIteratorRange<Entities>       ElementsRange;

public: // functions

  /// Contructor
  /// @param name of the component
  Region ( const std::string& name );

  /// Virtual destructor
  virtual ~Region();

  /// Get the class name
  static std::string type_name () { return "Region"; }

  // functions specific to the Region component

  /// create a Region component
  /// @param name of the region
  Region& create_region ( const std::string& name );

  /// create a Elements component, initialized to take connectivity data for the given type
  /// Set to refer to the supplied nodes
  /// @param element_type_name type of the elements
  /// @param nodes  location of the nodes the elements are linked with
  Elements& create_elements (const std::string& element_type_name, Dictionary& geometry);

  /// @return the number of elements stored in this region, including any subregions
  Uint recursive_elements_count(bool include_ghost_elems) const;

  /// @return the number of elements stored in this region, including any subregions
  ///         summed over all processors
  /// @todo remove ghost nodes from the count
  Uint global_elements_count(bool include_ghost_elems) const;

  /// @return the number of elements stored in this region, including any subregions
  template <typename Predicate>
    Uint recursive_filtered_elements_count(const Predicate& pred, bool include_ghost_elems) const;

  Uint recursive_nodes_count();

  /// @return nodes of the mesh
//  Dictionary& geometry_fields() const;

  /// @return non-modifiable range of elements that are searched for recursively
  /// for use with boost_foreach(const Elements& elements, region.elements_range() )
  ConstElementsRange elements_range() const;

  /// @return modifiable range of elements that are searched for recursively
  /// for use with boost_foreach(Elements& elements, region.elements_range() )
  ElementsRange elements_range();

private: // data

}; // Region

////////////////////////////////////////////////////////////////////////////////

template <typename Predicate>
inline Uint Region::recursive_filtered_elements_count(const Predicate& pred, bool include_ghost_elems) const
{
  Uint elem_count = 0;
  if (include_ghost_elems)
  {
    BOOST_FOREACH(const Entities& elements, common::find_components_recursively_with_filter<Entities>(*this,pred))
      elem_count += elements.size();
  }
  else
  {
    Uint nb_ghost;
    BOOST_FOREACH (const Entities& elements, common::find_components_recursively_with_filter<Entities>(*this,pred) )
    {
      elem_count += elements.size();

      // Count ghosts elements and subtract
      nb_ghost = 0;
      for (Uint e=0; e<elements.size(); ++e)
        if (elements.is_ghost(e))
          ++nb_ghost;
      elem_count -= nb_ghost;
    }
  }
  return elem_count;
}

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Region_hpp
