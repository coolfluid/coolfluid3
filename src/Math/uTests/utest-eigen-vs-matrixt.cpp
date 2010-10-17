// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Some benchmarkings for vector operations"

#include <boost/test/unit_test.hpp>
#include <boost/timer.hpp>

#include <Eigen/Dense>

#include "Math/RealMatrix.hpp"
#include "Math/RealVector.hpp"

using namespace std;
using namespace Eigen;

using namespace CF;
using namespace CF::Common;
using namespace CF::Math;

///////////////////////////////////////////////////////////////////////////////

struct nat
{
  double * cf_restrict data;
};

#define MSIZE 100000
#define LSIZE 4

BOOST_AUTO_TEST_SUITE( VectorBenchmarkSuite )

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( timed_dgemv_eigen_dynamic )
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

  boost::timer mtimer;

  for ( int i = 0; i < MSIZE; ++i )
    vc[i].noalias() = ma[i] * vb[i];

  cout << "[Eigen:Dyn]   dgemv elapsed time: " << mtimer.elapsed() << " seconds" << endl;
//  cout << vc[0] << endl;
}

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( timed_dgemv_eigen_fixed )
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

  boost::timer mtimer;

  for ( int i = 0; i < MSIZE; ++i )
    vc[i].noalias() = ma[i] * vb[i];

  cout << "[Eigen:Fixed] dgemv elapsed time: " << mtimer.elapsed() << " seconds" << endl;
//  cout << vc[0] << endl;
}

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( timed_dgemv_matrixt )
{
  std::vector< RealMatrix > ma;
  ma.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
  {
    ma[i].resize( LSIZE, LSIZE );
    ma[i] = 2.0;
  }

  std::vector< RealVector > vb;
  vb.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
  {
    vb[i].resize( LSIZE );
    vb[i] = 5.0;
  }

  std::vector< RealVector > vc;
  vc.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
  {
    vc[i].resize( LSIZE );
    vc[i] = 0.0;
  }

  boost::timer mtimer;

  for ( int i = 0; i < MSIZE; ++i )
    vc[i] = ma[i] * vb[i];

  cout << "[MatrixT]     dgemv elapsed time: " << mtimer.elapsed() << " seconds" << endl;
//  cout << vc[0] << endl;
}

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( timed_dgemv_native )
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

  boost::timer mtimer;

  for ( int e = 0; e < MSIZE; ++e )
    for ( int i = 0; i < LSIZE; ++i )
    {
      const unsigned n = LSIZE;
      for (Uint j = 0, k = i*n; j < n; ++j, ++k)
        vc[e].data[i] += ma[e].data[k] * vb[e].data[j];
    }

  cout << "[native]      dgemv elapsed time: " << mtimer.elapsed() << " seconds" << endl;
//  for ( int i = 0; i < LSIZE; ++i )
//    cout << vc[0].data[i] << endl;
}

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( timed_dgemm_eigen_dynamic )
{
  cout << "-------------------------" << endl;

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

  boost::timer mtimer;

  for ( int i = 0; i < MSIZE; ++i )
    mc[i].noalias() = ma[i] * mb[i];

  cout << "[Eigen:Dyn]   dgemm elapsed time: " << mtimer.elapsed() << " seconds" << endl;
//  cout << mc[0] << endl;
}

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( timed_dgemm_eigen_fixed )
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

  boost::timer mtimer;

  for ( int i = 0; i < MSIZE; ++i )
    mc[i].noalias() = ma[i] * mb[i];

  cout << "[Eigen:Fixed] dgemm elapsed time: " << mtimer.elapsed() << " seconds" << endl;
//  cout << mc[0] << endl;
}

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( timed_dgemm_matrixt )
{
  std::vector< RealMatrix > ma;
  ma.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
  {
    ma[i].resize( LSIZE, LSIZE );
    ma[i] = 2.0;
  }

  std::vector< RealMatrix > mb;
  mb.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
  {
    mb[i].resize( LSIZE, LSIZE );
    mb[i] = 7.0;
  }

  std::vector< RealMatrix > mc;
  mc.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
  {
    mc[i].resize( LSIZE, LSIZE );
    mc[i] = 0.0;
  }

  boost::timer mtimer;

  for ( int i = 0; i < MSIZE; ++i )
    mc[i] = ma[i] * mb[i];

  cout << "[MatrixT]     dgemm elapsed time: " << mtimer.elapsed() << " seconds" << endl;
//  cout << mc[0];
}

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( timed_dgemm_native )
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

  boost::timer mtimer;

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

  cout << "[native]      dgemv elapsed time: " << mtimer.elapsed() << " seconds" << endl;
//  for ( int i = 0; i < LSIZE; ++i )
//  {
//    for ( int j = 0; j < LSIZE; ++j )
//      cout << mc[0].data[i*LSIZE + j] << " ";
//    cout << endl;
//  }
}

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
