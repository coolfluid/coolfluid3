// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tetrahedral mesh testing on CGAL generated meshes"

#include <boost/test/unit_test.hpp>

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/mpl/for_each.hpp>

#include "common/FindComponents.hpp"
#include "common/Log.hpp"

#include "mesh/Connectivity.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "common/Table.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"

#include "mesh/LagrangeP1/Tetra3D.hpp"
#include "mesh/ElementTypes.hpp"

#include "CGAL/ImplicitFunctionMesh.hpp"


using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::CGAL;

//////////////////////////////////////////////////////////////////////////////

/// Use a global fixture, so mesh creation happens only once
struct GlobalFixture {

  GlobalFixture() {
    if(!sphere) {
      sphere = allocate_component<Mesh>("sphere");
      MeshParameters params;
      create_mesh(SphereFunction(1.), *sphere, params);
      sphere->write_mesh(URI("sphere.vtk"));
    }
  }

  static boost::shared_ptr< Mesh > sphere;
};

boost::shared_ptr< Mesh > GlobalFixture::sphere = boost::shared_ptr< Mesh >();

//////////////////////////////////////////////////////////////////////////////

/// Fixture for each test
struct TetraSFFixture
{
  TetraSFFixture() : sphere(*GlobalFixture::sphere) {}
  const Mesh& sphere;
};

//////////////////////////////////////////////////////////////////////////////

/// Applies a functor to all elements of the supplied region
/// Functor arguments: node list, element type
template<typename FunctorT>
struct LoopElems
{

  LoopElems( const Elements& aregion, FunctorT afunctor )
    : region(aregion),
      functor(afunctor)
  {}

  template < typename EType >
      void operator() ( EType& T )
  {

    /// @todo: Replace this with IsElementType when the conversion of elements is complete
    if( !IsElementType<EType>()(region.element_type()) )
      return;

    typename Table<Uint>::ArrayT const& conn_table = region.geometry_space().connectivity().array();
    const Table<Real>& coords = region.geometry_fields().coordinates();
    // loop on elements
    BOOST_FOREACH(const Table<Uint>::ConstRow& elem, conn_table)
    {
      typename EType::NodesT nodes;
      fill(nodes, coords, elem );
      functor(nodes, T);
    }
  }

  const Elements& region;
  FunctorT functor;
};

/// Looping over all elements in a range of regions
template<typename RangeT, typename FunctorT>
void loop_over_regions(const RangeT& range, FunctorT functor) {
  BOOST_FOREACH(const Elements& region, range) {
    boost::mpl::for_each<LagrangeP1::CellTypes>( LoopElems<FunctorT> ( region, functor ) );
  }
}

/// Simple test functor to compute the volume
struct VolumeFunctor {
  VolumeFunctor(Real& avolume) : volume(avolume) {}
  template<typename NodesT, typename ElementT>
  void operator()(const NodesT& nodes, const ElementT& element) {
    volume += ElementT::volume(nodes);
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
  loop_over_regions(find_components_recursively<Elements>(sphere), VolumeFunctor(volume));
  BOOST_CHECK_CLOSE(4.1627113937322715, volume, 0.1);
  CFinfo << "calculated volume: " << volume << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

