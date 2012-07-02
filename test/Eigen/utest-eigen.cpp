// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

    vb = VectorXd::LinSpaced(5, 2.0, 10.0);

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

  // functions

  /// This is how to modify a generic Eigen type
  /// If you don't do it this way, you cannot modify e.g. rows of matrices:
  /// @code
  /// Matrix<int,2,2> m;
  /// modify(m.row(0));  // This will now work
  /// @endcode
  template <typename Derived>
  void modify(Eigen::MatrixBase<Derived> const& m)
  {
    const_cast< MatrixBase<Derived>& >(m).setConstant(2);
  }

  // data

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
//  cout << vc << endl;
  vc = va.array() / vb.array();
//  cout << vc << endl;
}

BOOST_AUTO_TEST_CASE( VectorVectorOps )
{
  vc = va + vb;
//  cout << vc << endl;
  vc += vb;
//  cout << vc << endl;
}

BOOST_AUTO_TEST_CASE( MatrixMatrixOps )
{
  mc = ma * mb;
//  cout << mc << endl;
  mc += mb;
//  cout << mc << endl;
}

BOOST_AUTO_TEST_CASE( EigenMap )
{

  // Create an array
  std::vector<int> v(9);
  for (int i=0; i<9; ++i)
    v[i]=i;

  typedef Eigen::Matrix<int,3,3,Eigen::RowMajor> Matrix3x3_t;
  typedef Eigen::Matrix<int,1,9,Eigen::RowMajor> Matrix1x9_t;

  // Map the array to a 3x3 matrix
  Eigen::Map< Matrix3x3_t > em1(&v.front());
  // or equivalent:
  Matrix3x3_t::MapType em2(&v.front());

  // Map the array to a 1x9 matrix
  Eigen::Map< Matrix1x9_t > ev1(&v.front());
  // or equivalent
  Matrix1x9_t::MapType ev2(&v.front());


  // We can now modify the original array as if they were Eigen structures

  // set all elements of row1 to 2
  modify(em1.row(1));

  // Multiply entire matrix with 2
  ev1 *= 2;

//  cout << ev2 << endl << endl;

}



////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
