#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>

#include "Common/Log.hpp"
#include "Common/CoreEnv.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/ElementNodes.hpp"
#include "Mesh/P1/ElemTypes.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  /// Predicate class to teest if the region contains an element type
  struct IsElementRegion
  {
    bool operator()(const Component& component)
    {
      return !range_typed<CTable>(component).empty() && !range_typed<CElements>(component).empty();
    }
  };

  /// Predicate class to teest if the region contains an element type
  /// with a given dimensionality
  struct IsElementRegionDim
  {
    Uint dim;
    IsElementRegionDim (Uint adim) : dim(adim) {};
    bool operator()(const Component& component)
    {
      IsElementRegion isERegion;
      if ( isERegion(component) )
        return ( dim == dynamic_cast<const CRegion*>(&component)->get_elementType()->getDimensionality());
      else
        return false;
    }
  };

  /// Predicate class to test if the region contains a specific element type
  template < typename TYPE >
      struct IsElementRegionType
  {
    bool operator()(const Component& component)
    {
      if( !range_typed<CTable>(component).empty() &&
          !range_typed<CElements>(component).empty() )
      {
        const CElements& elemtype = get_component_typed<CElements>(component,IsComponentTrue());
        return IsElemType<TYPE>()( *elemtype.get_elementType() );
      }
      else
        return false;
    }
  }; // IsElementRegion

} // Mesh
} // CF

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

//template < typename Algo >
struct LoopRegionComputeVolumes
{
  CRegion& region;
  CArray&  coords;
  CArray&  volumes;

  LoopRegionComputeVolumes( CRegion& aregion, CArray& acoords, CArray& avolumes )
    : region(aregion),
      coords(acoords),
      volumes(avolumes)
  {}

  template < typename EType >
      void operator() ( EType T )
  {

    CFinfo << "Looping on " << EType::getClassName() << " ... ";

    if( T.getShape()          != region.elements_type().getShape()         ||
        T.getOrder()          != region.elements_type().getOrder()         ||
        T.getDimension()      != region.elements_type().getDimension()     ||
        T.getDimensionality() != region.elements_type().getDimensionality() )
    {

      CFinfo << "X" << CFendl;
      return;
    }

    CFinfo << "GO GO GO!" << CFendl;

    Uint e = 0;
    // loop on elements
    BOOST_FOREACH(const CTable::ConstRow& elem, region.get_connectivityTable()->table())
    {
      std::vector<CArray::Row> nodes;

      fill_node_list( std::inserter(nodes, nodes.begin()), coords, region, e );

      volumes.array()[e][0] = VolumeComputer< EType >::computeVolume( nodes );

      ++e;
    }
  }
}; // LoopRegionComputeVolumes


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

//template < typename Algo >
struct LoopRegionTypeComputeVolumes
{
  CMesh& mesh;

  LoopRegionTypeComputeVolumes( CMesh& amesh ) : mesh(amesh) {}

  template < typename EType >
      void operator() ( EType T )
  {
    CArray& coordinates = get_named_component_typed<CArray>(mesh, "coordinates");

    BOOST_FOREACH(CRegion& region, recursive_filtered_range_typed<CRegion>(mesh,IsElementRegionType<EType>()))
    {
      CFinfo << "Region [" << region.name() << "] of EType [" << EType::getClassName() << "]" << CFendl;

      const CElements& elemtype = get_component_typed<CElements>(region,IsComponentTrue());

      cf_assert ( IsElemType<EType>()( *elemtype.get_elementType() ) );

      // create an array to store the volumes
      CArray::Ptr volumes_ptr = region.get_child("volumes") ?
                                boost::dynamic_pointer_cast<CArray>(region.get_child("volumes")) :
                                region.create_component_type<CArray>("volumes");

      CArray::Array& volumes = volumes_ptr->array();
      volumes.resize( boost::extents[region.elements_count()][1]);

      Uint e = 0;
      // loop on elements
      BOOST_FOREACH(const CTable::ConstRow& elem, region.get_connectivityTable()->table())
      {
        std::vector<CArray::Row> nodes;

        fill_node_list( std::inserter(nodes, nodes.begin()), coordinates, region, e );

        volumes[e][0] = VolumeComputer< EType >::computeVolume( nodes );

        ++e;
      }
    }
  }

}; // LoopRegionComputeVolumes


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
  CFinfo << "Welcome to the COOLFLUID K3 solver!\n" << CFflush;

  CoreEnv::instance().initiate(argc, argv);

  
  try {

  // read the mesh
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
    
  // the file to read from
  boost::filesystem::path inputfile ("quadtriag.neu");
  
  // the mesh to store in
  CRoot::Ptr root = CRoot::create("root");
  CMesh::Ptr mesh = meshreader->create_mesh_from(inputfile);
  root->add_component(mesh);
    
  
  /// @todo automated way
  Uint volume_dim = 2;
  Uint surface_dim = volume_dim-1;
  
  CArray& coordinates = get_named_component_typed<CArray>(*mesh, "coordinates");
  
  // loop for all volume regions
  // loop on element types
  BOOST_FOREACH(CRegion& region, recursive_filtered_range_typed<CRegion>(*mesh,IsElementRegionDim(volume_dim)))
  {
    CArray::Ptr centres = region.create_component_type<CArray>("centres");
    centres->initialize(volume_dim);
    CArray::Buffer buffer = centres->create_buffer();
    buffer.increase_array_size(region.elements_count());
    
    // loop on elements
    BOOST_FOREACH(const CTable::ConstRow& elem, region.get_connectivityTable()->table())
    {
      
      // compute centre
      RealVector centre(0.0,volume_dim);
      BOOST_FOREACH(const Uint node, elem)
          centre += RealVector( coordinates.array()[node] );
      buffer.add_row_directly(centre);
    }
  }

  // loop for all surface regions
  // loop on element types
  BOOST_FOREACH(CRegion& region, recursive_filtered_range_typed<CRegion>(*mesh,IsElementRegionDim(surface_dim)))
  {
    CArray::Ptr centres = region.create_component_type<CArray>("centres");
    centres->initialize(volume_dim);
    CArray::Buffer buffer = centres->create_buffer();
    buffer.increase_array_size(region.elements_count());

    // loop on elements
    BOOST_FOREACH(const CTable::ConstRow& elem, region.get_connectivityTable()->table())
    {
      
      // compute centre
      RealVector centre(0.0,volume_dim);
      BOOST_FOREACH(const Uint node, elem)
        centre += RealVector( coordinates.array()[node] );
      buffer.add_row_directly(centre);
    }
  }


  //--------------------------------------------------------------------------

  // loop for all volume regions
  // loop on element types
  BOOST_FOREACH(CRegion& region, recursive_filtered_range_typed<CRegion>(*mesh,IsElementRegionDim(volume_dim)))
  {
    CFinfo << "Region " << region.name() << CFendl;

    // create an array to store the volumes
    CArray::Ptr volumes = region.get_child("volumes") ?
                          boost::dynamic_pointer_cast<CArray>(region.get_child("volumes")) :
                          region.create_component_type<CArray>("volumes");

    CArray::Array& vols = volumes->array();
    vols.resize( boost::extents[region.elements_count()][1]);

    boost::mpl::for_each<P1::ElemTypes>( LoopRegionComputeVolumes ( region, coordinates, *volumes ) );

  }

  //--------------------------------------------------------------------------

  // loop on element types
  boost::mpl::for_each<P1::ElemTypes>( LoopRegionTypeComputeVolumes ( *mesh ) );

  //--------------------------------------------------------------------------

  root->print_tree();

  //--------------------------------------------------------------------------

  }
  catch ( std::exception& ex )
  {
    CFerror << "Unhandled exception: " << ex.what() << CFendl;
  }
  catch ( ... )
  {
    CFerror << "Detected unknown exception" << CFendl;
  }

  CoreEnv::instance().terminate();

  return 0;
}


///////////////////////////////////////////////////////////////////////////////


