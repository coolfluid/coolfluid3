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

#define MSIZE 10000
#define LSIZE 10

BOOST_AUTO_TEST_SUITE( VectorBenchmarkSuite )

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( timed_dgemv_eigen )
{
  std::vector< MatrixXd > ma;
  ma.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    ma[i] = MatrixXd::Zero( LSIZE, LSIZE);

  std::vector< VectorXd > vb;
  vb.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    vb[i] = VectorXd::Zero( LSIZE );

  std::vector< VectorXd > vc;
  vc.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    vc[i] = VectorXd::Zero( LSIZE );

  boost::timer mtimer;

  for ( int i = 0; i < MSIZE; ++i )
    vc[i] = ma[i] * vb[i];

  cout << "[Eigen] elapsed time: " << mtimer.elapsed() << " seconds" << endl;
}

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( timed_dgemv_matrix )
{
  std::vector< RealMatrix > ma;
  ma.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    ma[i].resize( LSIZE, LSIZE);

  std::vector< RealVector > vb;
  vb.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    vb[i].resize( LSIZE );

  std::vector< RealVector > vc;
  vc.resize( MSIZE );
  for ( int i = 0; i < MSIZE; ++i )
    vc[i].resize( LSIZE );

  boost::timer mtimer;

  for ( int i = 0; i < MSIZE; ++i )
    vc[i] = ma[i] * vb[i];

  cout << "[MatrixT] elapsed time: " << mtimer.elapsed() << " seconds" << endl;
}

///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
