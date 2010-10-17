// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for Eigen"

#include <boost/test/unit_test.hpp>

#include <iostream>

#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

/// fixture for each test of Eigen
struct EigenFixture
{

  EigenFixture()
  {
    va.resize(5);
    va << 1.0, 2.0, 3.0, 5.0, 7.0;

    vb = VectorXd::LinSpaced(2.0, 10.0, 5);

    vc = VectorXd::Constant(5, 7.0);

    ma.resize(5,5);
    ma << 1.0 , 2.5 , 3.0 , 4.5 , 5.0 ,
          3.0 , 4.5 , 5.0 , 6.5 , 7.0 ,
          5.0 , 6.5 , 7.0 , 8.5 , 5.0 ,
          7.0 , 8.5 , 9.0 ,10.5 , 5.0 ,
          9.0 ,10.5 ,11.0 ,12.5 ,13.0 ;

    mb = MatrixXd::Identity(5,5);

    mc = MatrixXd::Zero(5,5);
  }

  VectorXd va;
  VectorXd vb;
  VectorXd vc;

  MatrixXd ma;
  MatrixXd mb;
  MatrixXd mc;

};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestEigen, EigenFixture )

//////////////////////////////////////////////////////////////////////////////

//  BOOST_CHECK( bool );
//  BOOST_CHECK_EQUAL(str, f.m_buffer);
//  BOOST_CHECK_CLOSE( v1, v2, 0.0001 );

BOOST_AUTO_TEST_CASE( VectorCoeffWiseOps )
{
  vc = va.array() * vb.array();
  cout << vc << endl;
  vc = va.array() / vb.array();
  cout << vc << endl;
}

BOOST_AUTO_TEST_CASE( VectorVectorOps )
{
  vc = va + vb;
  cout << vc << endl;
  vc += vb;
  cout << vc << endl;
}

BOOST_AUTO_TEST_CASE( MatrixMatrixOps )
{
  mc = ma * mb;
  cout << mc << endl;
  mc += mb;
  cout << mc << endl;
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
