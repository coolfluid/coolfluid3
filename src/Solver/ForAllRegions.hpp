#ifndef CF_ForAllRegions_hpp
#define CF_ForAllRegions_hpp

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"

#include "Common/ComponentPredicates.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  /// Predicate class to test if the region contains a specific element type
  template < typename TYPE >
      struct IsGeometryElementType
  {
    bool operator()(const CElements& component)
    {
      return IsElementType<TYPE>()( component.element_type() ) && component.has_tag("GeometryElements") ;
    }
  }; // IsElementRegion

  /// Predicate class to test if the region contains a specific element type
  template < typename TYPE >
  struct IsFieldElementType
  {
    bool operator()(const CElements& component)
    {
      return IsElementType<TYPE>()( component.element_type() ) && component.has_tag("FieldElements") ;
    }
  }; // IsElementRegion
  
  /// Predicate class to test if the region contains a specific element type
  template < typename TYPE >
  struct IsAnyElementType
  {
    bool operator()(const CElements& component)
    {
      return IsAnyElementType<TYPE>()( component.element_type() );
    }
  }; // IsElementRegion

template < typename Operation >
struct ForAllRegions
{
  CField& field1;
  CField& field2;

  ForAllRegions( CField& field1_in ) : field1(field1_in), field2(field1_in) {}

  ForAllRegions( CField& field1_in, CField& field2_in ) : field1(field1_in), field2(field2_in) {}

  template < typename EType >
      void operator() ( EType& T )
  {

    BOOST_FOREACH(CElements& elements, recursive_filtered_range_typed<CElements>(field1.support(),IsAnyElementType<EType>()))
    {
      CFinfo << "Elements [" << elements.name() << "] of EType [" << EType::type_name() << "]" << CFendl;

      Operation op ( elements );

      const CArray& coordinates = elements.coordinates();
      const CTable& ctable = elements.connectivity_table();

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

/////////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF






#endif // CF_ForAllRegions_hpp
