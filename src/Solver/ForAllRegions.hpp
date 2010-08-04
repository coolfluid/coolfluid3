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
    bool operator()(const CElements& component)
    {
      return IsElementType<TYPE>()( component.element_type() );
    }
  }; // IsElementRegion


template < typename Operation >
struct ForAllRegions
{
  CMesh& mesh;

  ForAllRegions( CMesh& amesh ) : mesh(amesh) {}

  template < typename EType >
      void operator() ( EType& T )
  {

    BOOST_FOREACH(CElements& region, recursive_filtered_range_typed<CElements>(mesh,IsElementRegionType<EType>()))
    {
      CFinfo << "Region [" << region.name() << "] of EType [" << EType::type_name() << "]" << CFendl;

      Operation op ( region );

      const CArray& coordinates = region.coordinates();
      const CTable& ctable = region.connectivity_table();

      // loop on elements
      const Uint elem_count = ctable.size();
      for ( Uint elem = 0; elem != elem_count; ++elem )
      {
        std::vector<RealVector> nodes;
        fill_node_list( std::inserter(nodes, nodes.begin()), coordinates, ctable, elem );
        op.template execute<EType>( elem, nodes );
      }
    }
  }

}; // ForAllRegions

} // Mesh
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_ForAllRegions_hpp
