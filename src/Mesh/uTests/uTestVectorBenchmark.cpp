// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Some benchmarkings for vector operations"

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Math;
using namespace CF::Mesh;

struct VectorBenchmarkFixture : Tools::Testing::TimedTestFixture
{
  static CMesh::Ptr grid_2d;
};

CMesh::Ptr VectorBenchmarkFixture::grid_2d;

/// Calculates the centroid of all centroids over a set of quads
template<typename VectorType>
void centroid_2d(const CTable::ArrayT& connectivity, const CArray::ArrayT& coords, VectorType c0, VectorType c1, VectorType c2, VectorType c3, VectorType& result)
{
  const Uint nb_elem = connectivity.size();
  
  result[XX] = 0.;
  result[YY] = 0.;
  
  for(Uint elem = 0; elem != nb_elem; ++elem)
  {
    const CTable::ConstRow row = connectivity[elem];
    
    const CArray::ConstRow crow0 = coords[row[0]];
    c0[XX] = crow0[XX]; c0[YY] = crow0[YY];
    
    const CArray::ConstRow crow1 = coords[row[1]];
    c1[XX] = crow1[XX]; c1[YY] = crow1[YY];
    
    const CArray::ConstRow crow2 = coords[row[2]];
    c2[XX] = crow2[XX]; c2[YY] = crow2[YY];
    
    const CArray::ConstRow crow3 = coords[row[3]];
    c3[XX] = crow3[XX]; c3[YY] = crow3[YY];
    
    result += 0.25*(c0 + c1 + c2 + c3);
  }
  
  result /= nb_elem;
}

BOOST_AUTO_TEST_SUITE( VectorBenchmarkSuite )

// Must be run  before the next tests
BOOST_FIXTURE_TEST_CASE( CreateMesh, VectorBenchmarkFixture )
{
  grid_2d.reset(new CMesh("big_grid"));
  Tools::MeshGeneration::create_rectangle(*grid_2d, 1., 1., 8000, 8000);
}

BOOST_FIXTURE_TEST_CASE( RealVector2D, VectorBenchmarkFixture )
{
  RealVector c0(2);
  RealVector c1(2);
  RealVector c2(2);
  RealVector c3(2);
  RealVector result(2);
  
  centroid_2d(recursive_get_component_typed<CTable>(*grid_2d, IsComponentTrue()).array(), recursive_get_component_typed<CArray>(*grid_2d, IsComponentTrue()).array(), c0, c1, c2, c3, result);
  
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
  
  centroid_2d(recursive_get_component_typed<CTable>(*grid_2d, IsComponentTrue()).array(), recursive_get_component_typed<CArray>(*grid_2d, IsComponentTrue()).array(), c0, c1, c2, c3, result);
  
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
  
  centroid_2d(recursive_get_component_typed<CTable>(*grid_2d, IsComponentTrue()).array(), recursive_get_component_typed<CArray>(*grid_2d, IsComponentTrue()).array(), c0, c1, c2, c3, result);
  
  BOOST_CHECK_CLOSE(result[XX], 0.5, 1e-6);
  BOOST_CHECK_CLOSE(result[YY], 0.5, 1e-6);
}

BOOST_AUTO_TEST_SUITE_END()
