// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Some benchmarkings for vector operations"

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include "Common/EigenAssertions.hpp"
#include <Eigen/Dense>

#include "Common/CRoot.hpp"

#include "Mesh/BlockMesh/BlockData.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Geometry.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Math;
using namespace CF::Mesh;
using namespace CF::Mesh::BlockMesh;

struct VectorBenchmarkFixture : Tools::Testing::TimedTestFixture
{
  static CMesh::Ptr grid_2d;
  static CMesh::Ptr channel_3d;
};

CMesh::Ptr VectorBenchmarkFixture::grid_2d;
CMesh::Ptr VectorBenchmarkFixture::channel_3d;

/// Calculates the centroid of all centroids over a set of quads
template<typename VectorType>
void centroid_2d(const CTable<Uint>::ArrayT& connectivity, const CTable<Real>::ArrayT& coords, VectorType c0, VectorType c1, VectorType c2, VectorType c3, VectorType& result)
{
  const Uint nb_elem = connectivity.size();

  result[XX] = 0.;
  result[YY] = 0.;

  for(Uint elem = 0; elem != nb_elem; ++elem)
  {
    const CTable<Uint>::ConstRow row = connectivity[elem];

    const CTable<Real>::ConstRow crow0 = coords[row[0]];
    c0[XX] = crow0[XX]; c0[YY] = crow0[YY];

    const CTable<Real>::ConstRow crow1 = coords[row[1]];
    c1[XX] = crow1[XX]; c1[YY] = crow1[YY];

    const CTable<Real>::ConstRow crow2 = coords[row[2]];
    c2[XX] = crow2[XX]; c2[YY] = crow2[YY];

    const CTable<Real>::ConstRow crow3 = coords[row[3]];
    c3[XX] = crow3[XX]; c3[YY] = crow3[YY];

    result += 0.25*(c0 + c1 + c2 + c3);
  }

  result /= nb_elem;
}

/// Calculates the centroid of all centroids over a set of hexahedra
template<typename VectorType>
void centroid_3d(const CTable<Uint>::ArrayT& connectivity, const CTable<Real>::ArrayT& coords
    , VectorType c0
    , VectorType c1
    , VectorType c2
    , VectorType c3
    , VectorType c4
    , VectorType c5
    , VectorType c6
    , VectorType c7
    , VectorType& result)
{
  const Uint nb_elem = connectivity.size();

  result[XX] = 0.;
  result[YY] = 0.;
  result[ZZ] = 0.;

  for(Uint elem = 0; elem != nb_elem; ++elem)
  {
    const CTable<Uint>::ConstRow row = connectivity[elem];

    const CTable<Real>::ConstRow crow0 = coords[row[0]];
    c0[XX] = crow0[XX]; c0[YY] = crow0[YY]; c0[ZZ] = crow0[ZZ];

    const CTable<Real>::ConstRow crow1 = coords[row[1]];
    c1[XX] = crow1[XX]; c1[YY] = crow1[YY]; c1[ZZ] = crow1[ZZ];

    const CTable<Real>::ConstRow crow2 = coords[row[2]];
    c2[XX] = crow2[XX]; c2[YY] = crow2[YY]; c2[ZZ] = crow2[ZZ];

    const CTable<Real>::ConstRow crow3 = coords[row[3]];
    c3[XX] = crow3[XX]; c3[YY] = crow3[YY]; c3[ZZ] = crow3[ZZ];

    const CTable<Real>::ConstRow crow4 = coords[row[4]];
    c4[XX] = crow4[XX]; c4[YY] = crow4[YY]; c4[ZZ] = crow4[ZZ];

    const CTable<Real>::ConstRow crow5 = coords[row[5]];
    c5[XX] = crow5[XX]; c5[YY] = crow5[YY]; c5[ZZ] = crow5[ZZ];

    const CTable<Real>::ConstRow crow6 = coords[row[6]];
    c6[XX] = crow6[XX]; c6[YY] = crow6[YY]; c6[ZZ] = crow6[ZZ];

    const CTable<Real>::ConstRow crow7 = coords[row[7]];
    c7[XX] = crow7[XX]; c7[YY] = crow7[YY]; c7[ZZ] = crow7[ZZ];

    result += 0.125*(c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7);
  }

  result /= nb_elem;
}

BOOST_AUTO_TEST_SUITE( VectorBenchmarkSuite )

// Must be run  before the next tests
BOOST_FIXTURE_TEST_CASE( CreateMesh, VectorBenchmarkFixture )
{
  grid_2d = allocate_component<CMesh>("grid_2d");
  Tools::MeshGeneration::create_rectangle(*grid_2d, 1., 1., 2000, 2000);
  channel_3d = allocate_component<CMesh>("channel_3d");
  CRoot::Ptr root = CRoot::create("root");
  BlockData& block_data = root->create_component<BlockData>("block_data");
  Tools::MeshGeneration::create_channel_3d(block_data, 10., 0.5, 5., 160, 80, 120, 0.1);
  build_mesh(block_data, *channel_3d);
}

BOOST_FIXTURE_TEST_CASE( RealVector2D, VectorBenchmarkFixture )
{
  RealVector c0(2);
  RealVector c1(2);
  RealVector c2(2);
  RealVector c3(2);
  RealVector result(2);

  centroid_2d( find_component_recursively_with_filter<CElements>( *grid_2d, IsElementsVolume() ).node_connectivity().array(), grid_2d->geometry().coordinates().array(), c0, c1, c2, c3, result);

  BOOST_CHECK_CLOSE(result[XX], 0.5, 1e-6);
  BOOST_CHECK_CLOSE(result[YY], 0.5, 1e-6);
}

BOOST_FIXTURE_TEST_CASE( UblasVector2DStatic, VectorBenchmarkFixture )
{
  boost::numeric::ublas::c_vector<Real, 2> c0(2);
  boost::numeric::ublas::c_vector<Real, 2> c1(2);
  boost::numeric::ublas::c_vector<Real, 2> c2(2);
  boost::numeric::ublas::c_vector<Real, 2> c3(2);
  boost::numeric::ublas::c_vector<Real, 2> result(2);

  centroid_2d( find_component_recursively_with_filter<CElements>( *grid_2d, IsElementsVolume() ).node_connectivity().array(), grid_2d->geometry().coordinates().array(), c0, c1, c2, c3, result);

  BOOST_CHECK_CLOSE(result[XX], 0.5, 1e-6);
  BOOST_CHECK_CLOSE(result[YY], 0.5, 1e-6);
}

BOOST_FIXTURE_TEST_CASE( UblasVector2DDynamic, VectorBenchmarkFixture )
{
  boost::numeric::ublas::vector<Real> c0(2);
  boost::numeric::ublas::vector<Real> c1(2);
  boost::numeric::ublas::vector<Real> c2(2);
  boost::numeric::ublas::vector<Real> c3(2);
  boost::numeric::ublas::vector<Real> result(2);

  centroid_2d( find_component_recursively_with_filter<CElements>( *grid_2d, IsElementsVolume() ).node_connectivity().array(), grid_2d->geometry().coordinates().array(), c0, c1, c2, c3, result);

  BOOST_CHECK_CLOSE(result[XX], 0.5, 1e-6);
  BOOST_CHECK_CLOSE(result[YY], 0.5, 1e-6);
}

BOOST_FIXTURE_TEST_CASE( RealVector3D, VectorBenchmarkFixture )
{
  RealVector c0(3);
  RealVector c1(3);
  RealVector c2(3);
  RealVector c3(3);
  RealVector c4(3);
  RealVector c5(3);
  RealVector c6(3);
  RealVector c7(3);
  RealVector result(3);

  const CElements& elems = find_component_recursively_with_name<CElements>(*channel_3d, "CF.Mesh.SF.LagrangeP1.Hexa3D");
  const CTable<Real>& coords = elems.geometry().coordinates();

  centroid_3d(elems.node_connectivity().array(), coords.array(), c0, c1, c2, c3, c4, c5, c6, c7, result);

  BOOST_CHECK_CLOSE(result[XX], 5., 1e-6);
  BOOST_CHECK_SMALL(result[YY], 1e-6);
  BOOST_CHECK_CLOSE(result[ZZ], 2.5, 1e-6);
}

BOOST_FIXTURE_TEST_CASE( UblasVector3DStatic, VectorBenchmarkFixture )
{
  boost::numeric::ublas::c_vector<Real, 3> c0(3);
  boost::numeric::ublas::c_vector<Real, 3> c1(3);
  boost::numeric::ublas::c_vector<Real, 3> c2(3);
  boost::numeric::ublas::c_vector<Real, 3> c3(3);
  boost::numeric::ublas::c_vector<Real, 3> c4(3);
  boost::numeric::ublas::c_vector<Real, 3> c5(3);
  boost::numeric::ublas::c_vector<Real, 3> c6(3);
  boost::numeric::ublas::c_vector<Real, 3> c7(3);
  boost::numeric::ublas::c_vector<Real, 3> result(3);

  const CElements& elems = find_component_recursively_with_name<CElements>(*channel_3d, "CF.Mesh.LagrangeP1.Hexa3D");
  const CTable<Real>& coords = elems.geometry().coordinates();

  centroid_3d(elems.node_connectivity().array(), coords.array(), c0, c1, c2, c3, c4, c5, c6, c7, result);

  BOOST_CHECK_CLOSE(result[XX], 5., 1e-6);
  BOOST_CHECK_SMALL(result[YY], 1e-6);
  BOOST_CHECK_CLOSE(result[ZZ], 2.5, 1e-6);
}

BOOST_FIXTURE_TEST_CASE( UblasVector3DDynamic, VectorBenchmarkFixture )
{
  boost::numeric::ublas::vector<Real> c0(3);
  boost::numeric::ublas::vector<Real> c1(3);
  boost::numeric::ublas::vector<Real> c2(3);
  boost::numeric::ublas::vector<Real> c3(3);
  boost::numeric::ublas::vector<Real> c4(3);
  boost::numeric::ublas::vector<Real> c5(3);
  boost::numeric::ublas::vector<Real> c6(3);
  boost::numeric::ublas::vector<Real> c7(3);
  boost::numeric::ublas::vector<Real> result(3);

  const CElements& elems = find_component_recursively_with_name<CElements>(*channel_3d, "CF.Mesh.LagrangeP1.Hexa3D");
  const CTable<Real>& coords = elems.geometry().coordinates();

  centroid_3d(elems.node_connectivity().array(), coords.array(), c0, c1, c2, c3, c4, c5, c6, c7, result);

  BOOST_CHECK_CLOSE(result[XX], 5., 1e-6);
  BOOST_CHECK_SMALL(result[YY], 1e-6);
  BOOST_CHECK_CLOSE(result[ZZ], 2.5, 1e-6);
}

BOOST_FIXTURE_TEST_CASE( EigenVector2DStatic, VectorBenchmarkFixture )
{
  Eigen::Vector2d c0(2);
  Eigen::Vector2d c1(2);
  Eigen::Vector2d c2(2);
  Eigen::Vector2d c3(2);
  Eigen::Vector2d result(2);

  centroid_2d( find_component_recursively_with_filter<CElements>( *grid_2d, IsElementsVolume() ).node_connectivity().array(), grid_2d->geometry().coordinates().array(), c0, c1, c2, c3, result);

  BOOST_CHECK_CLOSE(result[XX], 0.5, 1e-6);
  BOOST_CHECK_CLOSE(result[YY], 0.5, 1e-6);
}

BOOST_FIXTURE_TEST_CASE( EigenVector2DDynamic, VectorBenchmarkFixture )
{
  Eigen::VectorXd c0(2);
  Eigen::VectorXd c1(2);
  Eigen::VectorXd c2(2);
  Eigen::VectorXd c3(2);
  Eigen::VectorXd result(2);

  centroid_2d( find_component_recursively_with_filter<CElements>( *grid_2d, IsElementsVolume() ).node_connectivity().array(), grid_2d->geometry().coordinates().array(), c0, c1, c2, c3, result);

  BOOST_CHECK_CLOSE(result[XX], 0.5, 1e-6);
  BOOST_CHECK_CLOSE(result[YY], 0.5, 1e-6);
}

BOOST_FIXTURE_TEST_CASE( EigenVector3DStatic, VectorBenchmarkFixture )
{
  Eigen::Vector3d c0(3);
  Eigen::Vector3d c1(3);
  Eigen::Vector3d c2(3);
  Eigen::Vector3d c3(3);
  Eigen::Vector3d c4(3);
  Eigen::Vector3d c5(3);
  Eigen::Vector3d c6(3);
  Eigen::Vector3d c7(3);
  Eigen::Vector3d result(3);

  const CElements& elems = find_component_recursively_with_name<CElements>(*channel_3d, "CF.Mesh.LagrangeP1.Hexa3D");
  const CTable<Real>& coords = elems.geometry().coordinates();

  centroid_3d(elems.node_connectivity().array(), coords.array(), c0, c1, c2, c3, c4, c5, c6, c7, result);

  BOOST_CHECK_CLOSE(result[XX], 5., 1e-6);
  BOOST_CHECK_SMALL(result[YY], 1e-6);
  BOOST_CHECK_CLOSE(result[ZZ], 2.5, 1e-6);
}

BOOST_FIXTURE_TEST_CASE( EigenVector3DDynamic, VectorBenchmarkFixture )
{
  Eigen::VectorXd c0(3);
  Eigen::VectorXd c1(3);
  Eigen::VectorXd c2(3);
  Eigen::VectorXd c3(3);
  Eigen::VectorXd c4(3);
  Eigen::VectorXd c5(3);
  Eigen::VectorXd c6(3);
  Eigen::VectorXd c7(3);
  Eigen::VectorXd result(3);

  const CElements& elems = find_component_recursively_with_name<CElements>(*channel_3d, "CF.Mesh.LagrangeP1.Hexa3D");
  const CTable<Real>& coords = elems.geometry().coordinates();

  centroid_3d(elems.node_connectivity().array(), coords.array(), c0, c1, c2, c3, c4, c5, c6, c7, result);

  BOOST_CHECK_CLOSE(result[XX], 5., 1e-6);
  BOOST_CHECK_SMALL(result[YY], 1e-6);
  BOOST_CHECK_CLOSE(result[ZZ], 2.5, 1e-6);
}

BOOST_AUTO_TEST_SUITE_END()
