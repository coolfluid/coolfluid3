// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Some benchmarkings for vector operations"

#include <boost/test/unit_test.hpp>

#include "common/EigenAssertions.hpp"
#include <Eigen/Dense>

#include "common/CF.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

using namespace std;
using namespace Eigen;

using namespace cf3;
using namespace cf3::common;

using namespace Tools::Testing;

///////////////////////////////////////////////////////////////////////////////

struct nat
{
  double * cf_restrict data;
};

#define MSIZE 1000000
#define LSIZE 4

BOOST_FIXTURE_TEST_SUITE( VectorBenchmarkSuite, TimedTestFixture )

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( dgemv_eigen_dynamic )
{
  std::vector< MatrixXd > ma;
  ma.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    ma[i] = MatrixXd::Constant( LSIZE, LSIZE, 2.0);

  std::vector< VectorXd > vb;
  vb.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    vb[i] = VectorXd::Constant( LSIZE, 5.0);

  std::vector< VectorXd > vc;
  vc.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    vc[i] = VectorXd::Constant( LSIZE, 0.0);

  restart_timer();

  for ( int i = 0; i < MSIZE; ++i )
    vc[i].noalias() = ma[i] * vb[i];
}

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( dgemv_eigen_fixed )
{
  typedef Matrix< double, LSIZE, LSIZE >  MatrixSd;
  typedef Matrix< double, LSIZE, 1     >  VectorSd;

  std::vector< MatrixSd > ma;
  ma.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    ma[i] = MatrixSd::Constant( LSIZE, LSIZE, 2.0);

  std::vector< VectorSd > vb;
  vb.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    vb[i] = VectorSd::Constant( LSIZE, 5.0 );

  std::vector< VectorSd > vc;
  vc.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    vc[i] = VectorSd::Constant( LSIZE, 0.0 );

  restart_timer();

  for ( int i = 0; i < MSIZE; ++i )
    vc[i].noalias() = ma[i] * vb[i];
}

///////////////////////////////////////////////////////////////////////////////

// BOOST_AUTO_TEST_CASE( dgemv_matrixt )
// {
//   std::vector< RealMatrix > ma;
//   ma.resize( MSIZE );
//   for ( int i = 0; i < MSIZE; ++i )
//   {
//     ma[i].resize( LSIZE, LSIZE );
//     ma[i] = 2.0;
//   }
//
//   std::vector< RealVector > vb;
//   vb.resize( MSIZE );
//   for ( int i = 0; i < MSIZE; ++i )
//   {
//     vb[i].resize( LSIZE );
//     vb[i] = 5.0;
//   }
//
//   std::vector< RealVector > vc;
//   vc.resize( MSIZE );
//   for ( int i = 0; i < MSIZE; ++i )
//   {
//     vc[i].resize( LSIZE );
//     vc[i] = 0.0;
//   }
//
//   restart_timer();
//
//   for ( int i = 0; i < MSIZE; ++i )
//     vc[i] = ma[i] * vb[i];
// }

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( dgemv_native )
{
  std::vector< nat > ma;
  ma.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
  {
    ma[i].data = new double [ LSIZE * LSIZE ];
    for ( int j = 0; j < LSIZE*LSIZE; ++j ) ma[i].data[j] = 2.0;
  }

  std::vector< nat > vb;
  vb.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
  {
    vb[i].data = new double [ LSIZE ];
    for ( int j = 0; j < LSIZE; ++j ) vb[i].data[j] = 5.0;
  }

  std::vector< nat > vc;
  vc.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
  {
      vc[i].data = new double [ LSIZE ];
      for ( int j = 0; j < LSIZE; ++j ) vc[i].data[j] = 0.0;
  }

  restart_timer();

  for ( int e = 0; e < MSIZE; ++e )
    for ( int i = 0; i < LSIZE; ++i )
    {
      const unsigned n = LSIZE;
      for (Uint j = 0, k = i*n; j < n; ++j, ++k)
        vc[e].data[i] += ma[e].data[k] * vb[e].data[j];
    }
}

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( dgemm_eigen_dynamic )
{
  std::vector< MatrixXd > ma;
  ma.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    ma[i] = MatrixXd::Constant( LSIZE, LSIZE, 2.0);

  std::vector< MatrixXd > mb;
  mb.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    mb[i] = MatrixXd::Constant( LSIZE, LSIZE, 7.0);

  std::vector< MatrixXd > mc;
  mc.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    mc[i] = MatrixXd::Constant( LSIZE, LSIZE, 0.0);

  restart_timer();

  for ( int i = 0; i < MSIZE; ++i )
    mc[i].noalias() = ma[i] * mb[i];
}

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( dgemm_eigen_fixed )
{
  typedef Matrix< double, LSIZE, LSIZE >  MatrixSd;

  std::vector< MatrixSd > ma;
  ma.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    ma[i] = MatrixSd::Constant( LSIZE, LSIZE, 2.0);

  std::vector< MatrixSd > mb;
  mb.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    mb[i] = MatrixSd::Constant( LSIZE, LSIZE, 7.0);

  std::vector< MatrixSd > mc;
  mc.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    mc[i] = MatrixSd::Constant( LSIZE, LSIZE, 0.0);

  restart_timer();

  for ( int i = 0; i < MSIZE; ++i )
    mc[i].noalias() = ma[i] * mb[i];
}

///////////////////////////////////////////////////////////////////////////////

// BOOST_AUTO_TEST_CASE( dgemm_matrixt )
// {
//   std::vector< RealMatrix > ma;
//   ma.resize( MSIZE );
//   for ( int i = 0; i < MSIZE; ++i )
//   {
//     ma[i].resize( LSIZE, LSIZE );
//     ma[i] = 2.0;
//   }
//
//   std::vector< RealMatrix > mb;
//   mb.resize( MSIZE );
//   for ( int i = 0; i < MSIZE; ++i )
//   {
//     mb[i].resize( LSIZE, LSIZE );
//     mb[i] = 7.0;
//   }
//
//   std::vector< RealMatrix > mc;
//   mc.resize( MSIZE );
//   for ( int i = 0; i < MSIZE; ++i )
//   {
//     mc[i].resize( LSIZE, LSIZE );
//     mc[i] = 0.0;
//   }
//
//   restart_timer();
//
//   for ( int i = 0; i < MSIZE; ++i )
//     mc[i] = ma[i] * mb[i];
// }

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( dgemm_native )
{
  std::vector< nat > ma;
  ma.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
  {
    ma[i].data = new double [ LSIZE * LSIZE ];
    for ( int j = 0; j < LSIZE*LSIZE; ++j ) ma[i].data[j] = 2.0;
  }

  std::vector< nat > mb;
  mb.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
  {
    mb[i].data = new double [ LSIZE * LSIZE ];
    for ( int j = 0; j < LSIZE*LSIZE; ++j ) mb[i].data[j] = 7.0;
  }

  std::vector< nat > mc;
  mc.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
  {
    mc[i].data = new double [ LSIZE * LSIZE ];
    for ( int j = 0; j < LSIZE*LSIZE; ++j ) mc[i].data[j] = 0.0;
  }

  restart_timer();

  for ( int e = 0; e < MSIZE; ++e )
  {
    const size_t nc = LSIZE;
    const size_t m  = LSIZE;
    const size_t n  = LSIZE;

    const nat& A = ma[e];
    const nat& B = mb[e];
          nat& C = mc[e];

    for (size_t i = 0; i < m; ++i)
    {
      const size_t jstart = i*n;
      const size_t jmax = jstart + n;
      const size_t kstart = i*nc;
      size_t count = 0;
      size_t k = 0;
      while(count < nc)
      {
        double sum = 0.;
        for(size_t j = jstart; j < jmax; ++j)
        {
          sum += A.data[j] * B.data[k];
          k += nc;
        }
        C.data[kstart + count] = sum;
        k = ++count;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
