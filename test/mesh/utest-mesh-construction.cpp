// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh construction"

#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Group.hpp"

#include "mesh/Connectivity.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"
#include "mesh/Field.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/ElementConnectivity.hpp"

#include "mesh/MeshWriter.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

class ElementIterator :
    public boost::iterator_facade<ElementIterator,  // iterator
                                  Entity,                     // Value
                                  boost::bidirectional_traversal_tag, // search direction
                                  Entity                     // return type of dereference
                                 >
{
  typedef boost::iterator_facade<ElementIterator,
                                 Entity,
                                 boost::random_access_traversal_tag,
                                 Entity
                                > BaseT;
public:

  typedef BaseT::difference_type difference_type;

  /// Construct an iterator over the given set of components.
  /// If endIterator is true, the iterator is intialized
  /// at the end of the range, otherwise at the beginning.
  explicit ElementIterator(Entities& entities,
                           const Uint element_idx=0)
    : element(entities,element_idx) {}

private:
  friend class boost::iterator_core_access;

  bool equal(ElementIterator const& other) const { return (element == other.element); }

  void increment()
  {
    ++element.idx;
  }

  void decrement()
  {
    --element.idx;
  }

  void advance(const difference_type n) { element.idx += n; }

  template <typename T2>
  difference_type distance_to(ElementIterator const& other) const
  {
    return other.element.idx - element.idx;
  }

public:

  /// dereferencing
  const Entity& dereference() const { return element; }

private:

  Entity element;
};

boost::iterator_range<ElementIterator> make_range(Entities& entities)
{
  ElementIterator begin(entities);
  return boost::make_iterator_range(begin,begin+entities.size());
}



struct Mesh_API SpaceElement
{

  SpaceElement(Space& component, const Uint index=0) :
    comp( &component ),
    idx(index)
  {
    cf3_assert(idx<comp->support().size());
  }

  Space* comp;
  Uint idx;

  /// return the elementType
  const ShapeFunction& element_type() const { return comp->shape_function(); }

  /// Const access to the coordinates
  Dictionary& dict() const { return comp->dict(); }

  Entity support() const { return Entity(comp->support(),idx); }

  Connectivity::ConstRow field_indices() const { return comp->connectivity()[idx]; }
  Uint glb_idx() const { return comp->support().glb_idx()[idx]; }
  Uint rank() const { return comp->support().rank()[idx]; }
  bool is_ghost() const { return comp->support().is_ghost(idx); }
  RealMatrix get_coordinates() const { return comp->get_coordinates(idx); }
  void put_coordinates(RealMatrix& coordinates) const { return comp->put_coordinates(coordinates,idx); }
  void allocate_coordinates(RealMatrix& coordinates) const { return comp->allocate_coordinates(coordinates); }
};


////////////////////////////////////////////////////////////////////////////////

struct MeshConstruction_Fixture
{
  /// common setup for each test case
  MeshConstruction_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~MeshConstruction_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  /// These are handy functions that should maybe be implemented somewhere easily accessible.

  /// create a Real vector with 2 coordinates
  RealVector create_coord(const Real& x, const Real& y) {
    RealVector coordVec(2);
    coordVec[XX]=x;
    coordVec[YY]=y;
    return coordVec;
  }

  /// create a Uint vector with 4 node ID's
  std::vector<Uint> create_quad(const Uint& A, const Uint& B, const Uint& C, const Uint& D) {
    Uint quad[] = {A,B,C,D};
    std::vector<Uint> quadVec;
    quadVec.assign(quad,quad+4);
    return quadVec;
  }

  /// create a Uint vector with 3 node ID's
  std::vector<Uint> create_triag(const Uint& A, const Uint& B, const Uint& C) {
    Uint triag[] = {A,B,C};
    std::vector<Uint> triagVec;
    triagVec.assign(triag,triag+3);
    return triagVec;
  }

  std::vector<Uint> create_quad_p2(const Uint A, const Uint B, const Uint C, const Uint D,
                                   const Uint E, const Uint F, const Uint G, const Uint H, const Uint I)
  {
    Uint quad[] = {A,B,C,D,E,F,G,H,I};
    std::vector<Uint> quadVec;
    quadVec.assign(quad,quad+9);
    return quadVec;
  }

  /// create a Uint vector with 3 node ID's
  std::vector<Uint> create_triag_p2(const Uint A, const Uint B, const Uint C,
                                    const Uint D, const Uint E, const Uint F)
  {
    Uint triag[] = {A,B,C,D,E,F};
    std::vector<Uint> triagVec;
    triagVec.assign(triag,triag+6);
    return triagVec;
  }

  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshConstruction_TestSuite, MeshConstruction_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( P1_2D_MeshConstruction )
{
  AssertionManager::instance().AssertionThrows = true;

  const Uint dim=2;

  // Create root and mesh component
  Component& root = Core::instance().root();

  Mesh& mesh = *root.create_component<Mesh>( "mesh" ) ;

  // create regions
  Region& superRegion = mesh.topology().create_region("superRegion");
  Dictionary& nodes = mesh.geometry_fields();
  mesh.initialize_nodes(0,dim);
  BOOST_CHECK_EQUAL(nodes.coordinates().row_size() , dim);

  Elements& quadRegion = superRegion.create_elements("cf3.mesh.LagrangeP1.Quad2D",nodes);
  Elements& triagRegion = superRegion.create_elements("cf3.mesh.LagrangeP1.Triag2D",nodes);

  Table<Uint>::Buffer qTableBuffer = quadRegion.geometry_space().connectivity().create_buffer();
  Table<Uint>::Buffer tTableBuffer = triagRegion.geometry_space().connectivity().create_buffer();
  Table<Real>::Buffer coordinatesBuffer = nodes.coordinates().create_buffer();

  //  Mesh of quads and triangles with node and element numbering:
  //
  //    5----4----6
  //    |    |\ 3 |
  //    |    | \  |
  //    | 1  |  \ |
  //    |    | 2 \|
  //    3----2----7
  //    |    |\ 1 |
  //    |    | \  |
  //    | 0  |  \ |
  //    |    | 0 \|
  //    0----1----8

  // fill coordinates in the buffer
  coordinatesBuffer.add_row(create_coord( 0.0 , 0.0 ));  // 0
  coordinatesBuffer.add_row(create_coord( 1.0 , 0.0 ));  // 1
  coordinatesBuffer.add_row(create_coord( 1.0 , 1.0 ));  // 2
  coordinatesBuffer.add_row(create_coord( 0.0 , 1.0 ));  // 3
  coordinatesBuffer.add_row(create_coord( 1.0 , 2.0 ));  // 4
  coordinatesBuffer.add_row(create_coord( 0.0 , 2.0 ));  // 5
  coordinatesBuffer.add_row(create_coord( 2.0 , 2.0 ));  // 6
  coordinatesBuffer.add_row(create_coord( 2.0 , 1.0 ));  // 7
  coordinatesBuffer.add_row(create_coord( 2.0 , 0.0 ));  // 8

  // fill connectivity in the buffer
  qTableBuffer.add_row(create_quad( 0 , 1 , 2 , 3 ));
  qTableBuffer.add_row(create_quad( 3 , 2 , 4 , 5 ));

  tTableBuffer.add_row(create_triag( 1 , 8 , 2 ));
  tTableBuffer.add_row(create_triag( 8 , 7 , 2 ));
  tTableBuffer.add_row(create_triag( 2 , 7 , 4 ));
  tTableBuffer.add_row(create_triag( 7 , 6 , 4 ));

  // flush buffers into the table.
  // This causes the table and array to be resized and filled.
  coordinatesBuffer.flush();
  qTableBuffer.flush();
  tTableBuffer.flush();

  nodes.resize(nodes.coordinates().size());
  quadRegion.resize(quadRegion.size());
  triagRegion.resize(triagRegion.size());

  mesh.raise_mesh_loaded();
  BOOST_CHECK(true);

  // check if coordinates match
  Uint elem=1;
  Uint node=2;

  Table<Uint>::ConstRow nodesRef = triagRegion.geometry_space().connectivity()[elem];
  Table<Real>::Row coordRef = triagRegion.geometry_fields().coordinates()[nodesRef[node]];
  BOOST_CHECK_EQUAL(coordRef[0],1.0);
  BOOST_CHECK_EQUAL(coordRef[1],1.0);

  // calculate all volumes of a region
  BOOST_FOREACH( Elements& region, find_components_recursively<Elements>(superRegion))
  {
    const ElementType& elementType = region.element_type();
    const Uint nbRows = region.geometry_space().connectivity().size();
    std::vector<Real> volumes(nbRows);
    const Table<Real>& region_coordinates = region.geometry_fields().coordinates();
    const Table<Uint>& region_connTable = region.geometry_space().connectivity();

    // the loop
    RealMatrix elementCoordinates(elementType.nb_nodes(), elementType.dimension());
    for (Uint iElem=0; iElem<nbRows; ++iElem)
    {
      fill(elementCoordinates, region_coordinates, region_connTable[iElem]);

      volumes[iElem]=elementType.volume(elementCoordinates);

      // check
      if(elementType.shape() == GeoShape::QUAD)
        BOOST_CHECK_EQUAL(volumes[iElem],1.0);
      if(elementType.shape() == GeoShape::TRIAG)
        BOOST_CHECK_EQUAL(volumes[iElem],0.5);
    }
  }

//  BOOST_FOREACH(Table<Real>::Row node , elem_coord)
//  {
//    CFinfo << "node = ";
//    for (Uint j=0; j<node.size(); j++) {
//      CFinfo << node[j] << " ";
//    }
//    CFinfo << "\n" << CFflush;
//  }

  boost::shared_ptr< MeshWriter > meshwriter = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  meshwriter->write_from_to(mesh,"p1-mesh.msh");


  Entity entity(quadRegion,0);


  SpaceElement space_elem(quadRegion.space(nodes),0);
  BOOST_CHECK_EQUAL( space_elem.field_indices()[0] , 0 );

  RealMatrix geom_coords  = space_elem.support().get_coordinates();
  RealMatrix space_coords = space_elem.get_coordinates();

  BOOST_CHECK( geom_coords == space_coords );


  for(SpaceElement elem(quadRegion.space(nodes)); elem.idx<quadRegion.size(); ++elem.idx)
  {
    CFinfo << "indices = ";
    boost_foreach(Uint field_idx, elem.field_indices())
      CFinfo << field_idx << " ";
    CFinfo << CFendl;
  }
  ElementIterator it(quadRegion);
  boost_foreach( const Entity& entity, make_range(quadRegion) )
  {
    std::cout << entity << std::endl;
  }

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( P2_2D_MeshConstruction )
{
  AssertionManager::instance().AssertionThrows = true;

  const Uint dim=2;

  // Create root and mesh component
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));

  Mesh& mesh = *root->create_component<Mesh>( "mesh" );

  // create regions
  Region& superRegion = mesh.topology().create_region("superRegion");
  Dictionary& nodes = mesh.geometry_fields();
  mesh.initialize_nodes(0,dim);
  BOOST_CHECK_EQUAL(nodes.coordinates().row_size() , dim);
  Elements& quadRegion = superRegion.create_elements("cf3.mesh.LagrangeP2.Quad2D",nodes);
  Elements& triagRegion = superRegion.create_elements("cf3.mesh.LagrangeP2.Triag2D",nodes);

  Table<Uint>::Buffer qTableBuffer = quadRegion.geometry_space().connectivity().create_buffer();
  Table<Uint>::Buffer tTableBuffer = triagRegion.geometry_space().connectivity().create_buffer();
  Table<Real>::Buffer coordinatesBuffer = nodes.coordinates().create_buffer();

  //  Mesh of quads and triangles with node numbering and element numbering in brackets:
  //
  //  Y ^
  //    |
  //  2 -  5--15---4--24--6
  //    |  |       |\     |
  //    |  |       | \ (3)|
  // 1.5-  16 17  14  22 23
  //    |  |       |   \  |
  //    |  |  (1)  |(2) \ |
  //    |  |       |     \|
  //  1 -  3--11---2--21--7
  //    |  |       |\     |
  //    |  |       | \ (1)|
  // 0.5-  12 13  10  19 20
  //    |  |       |   \  |
  //    |  |  (0)  | (0)\ |
  //    |  |       |     \|
  //  0 -  0---9---1--18--8
  //    |
  //    o--|---|---|--|--|------> X
  //       0  0.5  1 1.5  2

  // fill coordinates in the buffer of the P1 mesh
  coordinatesBuffer.add_row(create_coord( 0.0 , 0.0 ));  // 0
  coordinatesBuffer.add_row(create_coord( 1.0 , 0.0 ));  // 1
  coordinatesBuffer.add_row(create_coord( 1.0 , 1.0 ));  // 2
  coordinatesBuffer.add_row(create_coord( 0.0 , 1.0 ));  // 3
  coordinatesBuffer.add_row(create_coord( 1.0 , 2.0 ));  // 4
  coordinatesBuffer.add_row(create_coord( 0.0 , 2.0 ));  // 5
  coordinatesBuffer.add_row(create_coord( 2.0 , 2.0 ));  // 6
  coordinatesBuffer.add_row(create_coord( 2.0 , 1.0 ));  // 7
  coordinatesBuffer.add_row(create_coord( 2.0 , 0.0 ));  // 8

  // enrich
  coordinatesBuffer.add_row(create_coord( 0.5 , 0.0 ));  // 9
  coordinatesBuffer.add_row(create_coord( 1.0 , 0.5 ));  // 10
  coordinatesBuffer.add_row(create_coord( 0.5 , 1.0 ));  // 11
  coordinatesBuffer.add_row(create_coord( 0.0 , 0.5 ));  // 12
  coordinatesBuffer.add_row(create_coord( 0.5 , 0.5 ));  // 13
  coordinatesBuffer.add_row(create_coord( 1.0 , 1.5 ));  // 14
  coordinatesBuffer.add_row(create_coord( 0.5 , 2.0 ));  // 15
  coordinatesBuffer.add_row(create_coord( 0.0 , 1.5 ));  // 16
  coordinatesBuffer.add_row(create_coord( 0.5 , 1.5 ));  // 17
  coordinatesBuffer.add_row(create_coord( 1.5 , 0.0 ));  // 18
  coordinatesBuffer.add_row(create_coord( 1.5 , 0.5 ));  // 19
  coordinatesBuffer.add_row(create_coord( 2.0 , 0.5 ));  // 20
  coordinatesBuffer.add_row(create_coord( 1.5 , 1.0 ));  // 21
  coordinatesBuffer.add_row(create_coord( 1.5 , 1.5 ));  // 22
  coordinatesBuffer.add_row(create_coord( 2.0 , 1.5 ));  // 23
  coordinatesBuffer.add_row(create_coord( 1.5 , 2.0 ));  // 24


  // enrich

  // fill connectivity in the buffer
  qTableBuffer.add_row(create_quad_p2( 0 , 1 , 2 , 3 , 9 , 10, 11, 12, 13 ));
  qTableBuffer.add_row(create_quad_p2( 3 , 2 , 4 , 5 , 11, 14, 15, 16, 17 ));

  tTableBuffer.add_row(create_triag_p2( 1 , 8 , 2 , 18 , 19 , 10 ));
  tTableBuffer.add_row(create_triag_p2( 8 , 7 , 2 , 20 , 21 , 19 ));
  tTableBuffer.add_row(create_triag_p2( 2 , 7 , 4 , 21 , 22 , 14 ));
  tTableBuffer.add_row(create_triag_p2( 7 , 6 , 4 , 23 , 24 , 22 ));

  // flush buffers into the table.
  // This causes the table and array to be resized and filled.
  coordinatesBuffer.flush();
  qTableBuffer.flush();
  tTableBuffer.flush();

  nodes.resize(nodes.coordinates().size());
  quadRegion.resize(quadRegion.size());
  triagRegion.resize(triagRegion.size());

  mesh.raise_mesh_loaded();
  BOOST_CHECK(true);

  // check if coordinates match
  Uint elem=1;
  Uint node=2;

  Table<Uint>::ConstRow nodesRef = triagRegion.geometry_space().connectivity()[elem];
  Table<Real>::Row coordRef = triagRegion.geometry_fields().coordinates()[nodesRef[node]];
  BOOST_CHECK_EQUAL(coordRef[0],1.0);
  BOOST_CHECK_EQUAL(coordRef[1],1.0);

//  // calculate all volumes of a region
//  BOOST_FOREACH( Elements& region, find_components_recursively<Elements>(superRegion))
//  {
//    const ElementType& elementType = region.element_type();
//    const Uint nbRows = region.geometry_space().connectivity().size();
//    std::vector<Real> volumes(nbRows);
//    const Table<Real>& region_coordinates = region.coordinates();
//    const Table<Uint>& region_connTable = region.geometry_space().connectivity();
//    // the loop
//    ElementType::NodesT elementCoordinates(elementType.nb_nodes(), elementType.dimension());
//    for (Uint iElem=0; iElem<nbRows; ++iElem)
//    {
//      elementCoordinates.fill(region_coordinates, region_connTable[iElem]);
//
//      volumes[iElem]=elementType.compute_volume(elementCoordinates);
//
//      // check
//      if(elementType.shape() == GeoShape::QUAD)
//        BOOST_CHECK_EQUAL(volumes[iElem],1.0);
//      if(elementType.shape() == GeoShape::TRIAG)
//        BOOST_CHECK_EQUAL(volumes[iElem],0.5);
//    }
//  }
//
//	//  BOOST_FOREACH(Table<Real>::Row node , elem_coord)
//	//  {
//	//    CFinfo << "node = ";
//	//    for (Uint j=0; j<node.size(); j++) {
//	//      CFinfo << node[j] << " ";
//	//    }
//	//    CFinfo << "\n" << CFflush;
//	//  }


  boost::shared_ptr< MeshWriter > meshwriter = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  meshwriter->write_from_to(mesh,"p2-mesh.msh");

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

