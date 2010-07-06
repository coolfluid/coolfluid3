#ifndef CF_ForAllRegions_hpp
#define CF_ForAllRegions_hpp

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/ElementNodes.hpp"

#include "Common/ComponentPredicates.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  /// Predicate class to test if the region contains a specific element type
  template < typename TYPE >
      struct IsElementRegionType
  {
    bool operator()(const Component& component)
    {
      if( !range_typed<CTable>(component).empty() &&
          !range_typed<CElements>(component).empty() )
      {
        const CElements& elemtype = get_component_typed<CElements>(component);
        return IsElemType<TYPE>()( *elemtype.get_elementType() );
      }
      else
        return false;
    }
  }; // IsElementRegion


template < typename Operation >
struct ForAllRegions
{
  CMesh& mesh;

  ForAllRegions( CMesh& amesh ) : mesh(amesh) {}

  template < typename EType >
      void operator() ( EType T )
  {

    BOOST_FOREACH(CRegion& region, recursive_filtered_range_typed<CRegion>(mesh,IsElementRegionType<EType>()))
    {
      CFinfo << "Region [" << region.name() << "] of EType [" << EType::type_name() << "]" << CFendl;

      Operation op ( region );

      const CElements& elemtype = get_component_typed<CElements>(region);

      cf_assert ( IsElemType<EType>()( *elemtype.get_elementType() ) );

      CArray& coordinates = get_named_component_typed<CArray>(mesh, "coordinates");

      // loop on elements
      for ( Uint elem = 0; elem < region.elements_count(); ++elem )
      {
        std::vector<CArray::Row> nodes;
        fill_node_list( std::inserter(nodes, nodes.begin()), coordinates, region, elem );
        op.template execute<EType>( elem, nodes );

      }
    }
  }

}; // ForAllRegions

} // Mesh
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_ForAllRegions_hpp
