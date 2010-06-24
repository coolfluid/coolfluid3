#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tetrahedral mesh testing on CGAL generated meshes"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/ComponentPredicates.hpp"
#include "Common/Log.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/ElementNodes.hpp"

#include "Mesh/LagrangeSF/TetraP1.hpp"

#include "Mesh/P1/ElemTypes.hpp"

#include "Mesh/CGAL/ImplicitFunctionMesh.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::CGAL;

//////////////////////////////////////////////////////////////////////////////

/// Use a global fixture, so mesh creation happens only once
struct GlobalFixture {

  GlobalFixture() {
    if(!sphere) {
      sphere.reset(new CMesh("sphere"));
      MeshParameters params;
      params.cell_size = 0.25;
      params.facet_distance = 0.05;
      create_mesh(SphereFunction(1.), *sphere, params);
    }
  }

  static CMesh::Ptr sphere;
};

CMesh::Ptr GlobalFixture::sphere = CMesh::Ptr();

//////////////////////////////////////////////////////////////////////////////

/// Fixture for each test
struct TetraSFFixture
{
  TetraSFFixture() : sphere(*GlobalFixture::sphere) {}
  const CMesh& sphere;
};

//////////////////////////////////////////////////////////////////////////////

/// Applies a functor to all elements of the supplied region
/// Functor arguments: node list, element type
template<typename FunctorT>
struct LoopElems
{

  LoopElems( const CRegion& aregion, const CArray& acoords, FunctorT& afunctor )
    : region(aregion),
      coords(acoords),
      functor(afunctor)
  {}

  template < typename EType >
      void operator() ( EType& T )
  {

    if( T.getShape()          != region.elements_type().getShape()         ||
        T.getOrder()          != region.elements_type().getOrder()         ||
        T.getDimension()      != region.elements_type().getDimension()     ||
        T.getDimensionality() != region.elements_type().getDimensionality() )
    return;

    CFinfo << "Looping on " << T.getClassName() << CFendl;

    const Uint elem_count = region.elements_count();
    cf_assert(elem_count);
    typename CTable::ConnectivityTable const& conn_table = region.get_connectivityTable()->table();
    // loop on elements
    BOOST_FOREACH(const CTable::ConstRow& elem, conn_table)
    {
      ElementNodeVector nodes;
      fill_node_list( std::inserter(nodes, nodes.begin()), coords, elem );
      functor(nodes, T);
    }
  }

  const CRegion& region;
  const CArray&  coords;
  FunctorT& functor;
};

/// Looping over all elements in a range of regions
template<typename RangeT, typename ArrayT, typename FunctorT>
void loop_over_regions(const RangeT& range, ArrayT& coordinates, FunctorT functor) {
  BOOST_FOREACH(const CRegion& region, range) {
    boost::mpl::for_each<P1::ElemTypes>( LoopElems<FunctorT> ( region, coordinates, functor ) );
  }
}

/// Simple test functor to compute the volume
struct VolumeFunctor {
  VolumeFunctor(Real& avolume) : volume(avolume) {}
  template<typename NodesT, typename ElementT>
  void operator()(const NodesT& nodes, const ElementT& element) {
    const RealVector mapped_coords = boost::assign::list_of(0.)(0.)(0.);
    volume += LagrangeSF::TetraP1::computeJacobianDeterminant(mapped_coords, nodes)/6.;
  }
  Real& volume;
};

//////////////////////////////////////////////////////////////////////////////

BOOST_GLOBAL_FIXTURE( GlobalFixture ) // creates a global fixture, and thus all meshes

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TetraSF, TetraSFFixture )

//////////////////////////////////////////////////////////////////////////////

/// Accumulate some statistics about the cell volumes
BOOST_AUTO_TEST_CASE( MeshStats )
{
  Real volume = 0.;
  loop_over_regions(range_typed<CRegion>(sphere), get_named_component_typed<CArray>(sphere, "coordinates"), VolumeFunctor(volume));
  CFinfo << "calculated volume: " << volume << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

