#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/ElementType.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct P1Elements_Fixture
{
  /// common setup for each test case
  P1Elements_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~P1Elements_Fixture()
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

  /// create a Real vector with 3 coordinates
  RealVector create_coord(const Real& x, const Real& y, const Real& z) {
    RealVector coordVec(3);
    coordVec[XX]=x;
    coordVec[YY]=y;
    coordVec[ZZ]=z;
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
  
  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( P1Elements_TestSuite, P1Elements_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( VolumeTest )
{
  std::cout << "VolumeTest" << std::endl;
  typedef boost::multi_array<Real,2> Array;
  typedef Array::subarray<1>::type Row;
  CElements::Ptr etype ( new CElements("element-type"));

  // P1-Line1D
  etype->set_elementType("P1-Line1D");
  boost::multi_array<Real,2> nodes_line1D (boost::extents[2][1]);
  nodes_line1D[0][XX] = 2.0;
  nodes_line1D[1][XX] = 1.0;
  std::vector<Row> nodes_line1D_vec;
  BOOST_FOREACH (const Row& node, nodes_line1D)
      nodes_line1D_vec.push_back(node);
  BOOST_CHECK_EQUAL(etype->get_elementType()->computeVolume(nodes_line1D_vec),1.);

  // P1-Line2D
  etype->set_elementType("P1-Line2D");
  boost::multi_array<Real,2> nodes_line2D (boost::extents[2][2]);
  nodes_line2D[0][XX] = 2.0;     nodes_line2D[0][YY] = 2.0;
  nodes_line2D[1][XX] = 1.0;     nodes_line2D[1][YY] = 1.0;
  std::vector<Row> nodes_line2D_vec;
  BOOST_FOREACH (const Row& node, nodes_line2D)
      nodes_line2D_vec.push_back(node);
  BOOST_CHECK_EQUAL(etype->get_elementType()->computeVolume(nodes_line2D_vec),std::sqrt(2.));

  // P1-Line3D
  etype->set_elementType("P1-Line3D");
  boost::multi_array<Real,2> nodes_line3D (boost::extents[2][3]);
  nodes_line3D[0][XX] = 2.0;     nodes_line3D[0][YY] = 2.0;     nodes_line3D[0][ZZ] = 2.0;
  nodes_line3D[1][XX] = 1.0;     nodes_line3D[1][YY] = 1.0;     nodes_line3D[1][ZZ] = 1.0;
  std::vector<Row> nodes_line3D_vec;
  BOOST_FOREACH (const Row& node, nodes_line3D)
      nodes_line3D_vec.push_back(node);
  BOOST_CHECK_EQUAL(etype->get_elementType()->computeVolume(nodes_line3D_vec),std::sqrt(3.));

  // P1-Triag2D
  etype->set_elementType("P1-Triag2D");
  boost::multi_array<Real,2> nodes_triag2D (boost::extents[3][2]);
  nodes_triag2D[0][XX] = 0.0;     nodes_triag2D[0][YY] = 0.0;
  nodes_triag2D[1][XX] = 1.0;     nodes_triag2D[1][YY] = 0.0;
  nodes_triag2D[2][XX] = 1.0;     nodes_triag2D[2][YY] = 1.0;
  std::vector<Row> nodes_triag2D_vec;
  BOOST_FOREACH (const Row& node, nodes_triag2D)
      nodes_triag2D_vec.push_back(node);
  BOOST_CHECK_EQUAL(etype->get_elementType()->computeVolume(nodes_triag2D_vec), 0.5);

  // P1-Triag3D
  etype->set_elementType("P1-Triag3D");
  boost::multi_array<Real,2> nodes_triag3D (boost::extents[3][3]);
  nodes_triag3D[0][XX] = 0.0;     nodes_triag3D[0][YY] = 0.0;     nodes_triag3D[0][ZZ] = 0.0;
  nodes_triag3D[1][XX] = 1.0;     nodes_triag3D[1][YY] = 0.0;     nodes_triag3D[1][ZZ] = 1.0;
  nodes_triag3D[2][XX] = 1.0;     nodes_triag3D[2][YY] = 1.0;     nodes_triag3D[2][ZZ] = 1.0;
  std::vector<Row> nodes_triag3D_vec;
  BOOST_FOREACH (const Row& node, nodes_triag3D)
      nodes_triag3D_vec.push_back(node);
  BOOST_CHECK_EQUAL(etype->get_elementType()->computeVolume(nodes_triag3D_vec), std::sqrt(2.)/2.);

  // P1-Quad2D
  etype->set_elementType("P1-Quad2D");
  boost::multi_array<Real,2> nodes_quad2D (boost::extents[4][2]);
  nodes_quad2D[0][XX] = 0.0;     nodes_quad2D[0][YY] = 0.0;
  nodes_quad2D[1][XX] = 1.0;     nodes_quad2D[1][YY] = 0.0;
  nodes_quad2D[2][XX] = 1.0;     nodes_quad2D[2][YY] = 1.0;
  nodes_quad2D[3][XX] = 0.0;     nodes_quad2D[3][YY] = 1.0;
  std::vector<Row> nodes_quad2D_vec;
  BOOST_FOREACH (const Row& node, nodes_quad2D)
      nodes_quad2D_vec.push_back(node);
  BOOST_CHECK_EQUAL(etype->get_elementType()->computeVolume(nodes_quad2D_vec), 1.0);

  // P1-Quad3D
  etype->set_elementType("P1-Quad3D");
  boost::multi_array<Real,2> nodes_quad3D (boost::extents[4][3]);
  nodes_quad3D[0][XX] = 0.0;     nodes_quad3D[0][YY] = 0.0;     nodes_quad3D[0][ZZ] = 0.0;
  nodes_quad3D[1][XX] = 1.0;     nodes_quad3D[1][YY] = 0.0;     nodes_quad3D[1][ZZ] = 1.0;
  nodes_quad3D[2][XX] = 1.0;     nodes_quad3D[2][YY] = 1.0;     nodes_quad3D[2][ZZ] = 1.0;
  nodes_quad3D[3][XX] = 0.0;     nodes_quad3D[3][YY] = 1.0;     nodes_quad3D[3][ZZ] = 0.0;
  std::vector<Row> nodes_quad3D_vec;
  BOOST_FOREACH (const Row& node, nodes_quad3D)
      nodes_quad3D_vec.push_back(node);
  BOOST_CHECK_EQUAL(etype->get_elementType()->computeVolume(nodes_quad3D_vec), std::sqrt(2.));

  // P1-Hexa3D
  etype->set_elementType("P1-Hexa3D");
  boost::multi_array<Real,2> nodes_hexa3D (boost::extents[8][3]);
  nodes_hexa3D[0][XX] = 0.0;     nodes_hexa3D[0][YY] = 0.0;     nodes_hexa3D[0][ZZ] = 0.0;
  nodes_hexa3D[1][XX] = 1.0;     nodes_hexa3D[1][YY] = 0.0;     nodes_hexa3D[1][ZZ] = 0.0;
  nodes_hexa3D[2][XX] = 1.0;     nodes_hexa3D[2][YY] = 1.0;     nodes_hexa3D[2][ZZ] = 0.0;
  nodes_hexa3D[3][XX] = 0.0;     nodes_hexa3D[3][YY] = 1.0;     nodes_hexa3D[3][ZZ] = 0.0;
  nodes_hexa3D[4][XX] = 0.0;     nodes_hexa3D[4][YY] = 0.0;     nodes_hexa3D[4][ZZ] = 1.0;
  nodes_hexa3D[5][XX] = 1.0;     nodes_hexa3D[5][YY] = 0.0;     nodes_hexa3D[5][ZZ] = 1.0;
  nodes_hexa3D[6][XX] = 1.0;     nodes_hexa3D[6][YY] = 1.0;     nodes_hexa3D[6][ZZ] = 1.0;
  nodes_hexa3D[7][XX] = 0.0;     nodes_hexa3D[7][YY] = 1.0;     nodes_hexa3D[7][ZZ] = 1.0;
  std::vector<Row> nodes_hexa3D_vec;
  BOOST_FOREACH (const Row& node, nodes_hexa3D)
      nodes_hexa3D_vec.push_back(node);
  BOOST_CHECK_EQUAL(etype->get_elementType()->computeVolume(nodes_hexa3D_vec), 1.);

}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

