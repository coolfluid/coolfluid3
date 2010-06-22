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

namespace CF
{
  namespace Mesh
  {
    class IsElementRegion
    {
    public:
      IsElementRegion () {}
      
      bool operator()(const Component& component)
      {
        return !range_typed<CTable>(component).empty() && !range_typed<CElements>(component).empty();
      }
      
    }; // IsElementRegion
    
    class IsElementRegionWithDimensionality
    {
    public:
      IsElementRegionWithDimensionality (Uint dim) : m_dim(dim) {}
      
      bool operator()(const Component& component)
      {
        if (m_isElementRegion(component))
        {
          return (m_dim == dynamic_cast<const CRegion*>(&component)->get_elementType()->getDimensionality());
        }
        else
        {
          return false;
        }
      }
      
    private:
      IsElementRegion m_isElementRegion;
      Uint m_dim;
      
    }; // IsElementRegion
  }
}

//------------------------------------------------------------------

//template < typename Algo >
struct LoopElems
{

  LoopElems( CRegion& aregion, CArray& acoords, CArray& avolumes )
    : region(aregion),
      coords(acoords),
      volumes(avolumes)
  {}

  template < typename EType >
      void operator() ( EType T )
  {

    if( T.getShape()          != region.elements_type().getShape()         ||
        T.getOrder()          != region.elements_type().getOrder()         ||
        T.getDimension()      != region.elements_type().getDimension()     ||
        T.getDimensionality() != region.elements_type().getDimensionality() )
    return;

    CFinfo << "Looping on " << T.getClassName() << CFendl;

    Uint e = 0;
    // loop on elements
    BOOST_FOREACH(const CTable::ConstRow& elem, region.get_connectivityTable()->get_table())
    {
      std::vector<CArray::Row> nodes;

      fill_node_list( std::inserter(nodes, nodes.begin()), coords, region, e );

      volumes.get_array()[e][0] = VolumeComputer< EType >::computeVolume( nodes );

      ++e;
    }
  }

  CRegion& region;
  CArray&  coords;
  CArray&  volumes;

};


void run_loop();

int main(int argc, char * argv[])
{
  CFinfo << "Welcome to the COOLFLUID K3 solver!\n" << CFflush;

  CoreEnv::instance().initiate(argc, argv);

  
  
  
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
    
  // the file to read from
  boost::filesystem::path inputfile ("quadtriag.neu");
  
  // the mesh to store in
  CRoot::Ptr root = CRoot::create("root");
  CMesh::Ptr mesh = meshreader->create_mesh_from(inputfile);
  root->add_component(mesh);
  
  //mesh->print_tree();
  
  
  /// @todo automated way
  Uint volume_dim = 2;
  Uint surface_dim = volume_dim-1;
  
  CArray& coordinates = get_named_component_typed<CArray>(*mesh, "coordinates");
  
  // loop for all volume regions
  // loop on element types
  BOOST_FOREACH(CRegion& region, recursive_filtered_range_typed<CRegion>(*mesh,IsElementRegionWithDimensionality(volume_dim)))
  {
    CArray::Ptr centres = region.create_component_type<CArray>("centres");
    centres->initialize(volume_dim);
    CArray::Buffer buffer = centres->create_buffer();
    buffer.increase_array_size(region.elements_count());
    
    // loop on elements
    BOOST_FOREACH(const CTable::ConstRow& elem, region.get_connectivityTable()->get_table())
    {
      
      // compute centre
      RealVector centre(0.0,volume_dim);
      BOOST_FOREACH(const Uint node, elem)
          centre += RealVector( coordinates.get_array()[node] );
      buffer.add_row_directly(centre);
    }
  }
  
  
  
  // loop for all surface regions
  // loop on element types
  BOOST_FOREACH(CRegion& region, recursive_filtered_range_typed<CRegion>(*mesh,IsElementRegionWithDimensionality(surface_dim)))
  {
    CArray::Ptr centres = region.create_component_type<CArray>("centres");
    centres->initialize(volume_dim);
    CArray::Buffer buffer = centres->create_buffer();
    buffer.increase_array_size(region.elements_count());

    // loop on elements
    BOOST_FOREACH(const CTable::ConstRow& elem, region.get_connectivityTable()->get_table())
    {
      
      // compute centre
      RealVector centre(0.0,volume_dim);
      BOOST_FOREACH(const Uint node, elem)
        centre += RealVector( coordinates.get_array()[node] );
      buffer.add_row_directly(centre);
    }
  }


  root->print_tree();


//--------------------------------------------------------------------------

  // loop for all volume regions
  // loop on element types
  BOOST_FOREACH(CRegion& region, recursive_filtered_range_typed<CRegion>(*mesh,IsLeafWithDimensionality(volume_dim)))
  {
    // create an array to store the volumes
    CArray::Ptr volumes = region.create_component_type<CArray>("volumes");
    CArray::Array& vols = volumes->get_array();
    vols.resize( boost::extents[region.elements_count()][1]);

    boost::mpl::for_each<P1::ElemTypes>( LoopElems ( region, coordinates, *volumes ) );

  }


  //--------------------------------------------------------------------------

	run_loop();

  //--------------------------------------------------------------------------

  CoreEnv::instance().terminate();

  return 0;
}


///////////////////////////////////////////////////////////////////////////////

struct ET {};

struct Triag : public ET
{
  void flux() { CFinfo << "Triag::flux()" << CFendl; }
};

struct Quad : public ET
{
  void flux() { CFinfo << "Quad::flux()" << CFendl; }
};

typedef boost::mpl::vector< Triag, Quad > ElemTypes;

///////////////////////////////////////////////////////////////////////////////

struct TermCollector
{
  template< typename T > void operator()( T aT )
  {
    aT.flux();
  }
};

///////////////////////////////////////////////////////////////////////////////

void run_loop()
{
  TermCollector tc;

  boost::mpl::for_each<ElemTypes>( tc );
}


