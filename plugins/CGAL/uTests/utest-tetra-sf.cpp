// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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

#include "Common/ComponentPredicates.hpp"
#include "Common/Log.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/ElementNodes.hpp"
#include "Mesh/CMeshWriter.hpp"

#include "Mesh/SF/Tetra3DLagrangeP1.hpp"
#include "Mesh/SF/Types.hpp"

#include "CGAL/ImplicitFunctionMesh.hpp"


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
      create_mesh(SphereFunction(1.), *sphere, params);
      CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
      boost::filesystem::path file_out("sphere.msh");
      meshwriter->write_from_to(sphere,file_out);
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

  LoopElems( const CElements& aregion, FunctorT afunctor )
    : region(aregion),
      functor(afunctor)
  {}

  template < typename EType >
      void operator() ( EType& T )
  {

    /// @todo: Replace this with IsElementType when the conversion of elements is complete
    if( !IsElementType<EType>()(region.element_type()) )
      return;

    typename CTable::ArrayT const& conn_table = region.connectivity_table().array();
    const CArray& coords = region.coordinates();
    // loop on elements
    BOOST_FOREACH(const CTable::ConstRow& elem, conn_table)
    {
      ElementNodeVector nodes;
      fill_node_list( nodes, coords, elem );
      functor(nodes, T);
    }
  }

  const CElements& region;
  FunctorT functor;
};

/// Looping over all elements in a range of regions
template<typename RangeT, typename FunctorT>
void loop_over_regions(const RangeT& range, FunctorT functor) {
  BOOST_FOREACH(const CElements& region, range) {
    boost::mpl::for_each<SF::Types>( LoopElems<FunctorT> ( region, functor ) );
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
  loop_over_regions(recursive_range_typed<CElements>(sphere), VolumeFunctor(volume));
  CFinfo << "calculated volume: " << volume << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

