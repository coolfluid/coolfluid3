// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for SF interpolation in matrix form"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Math/MatrixTypes.hpp"
#include "Mesh/LagrangeP1/Triag2D.hpp"

#include "Tools/Testing/Difference.hpp"
#include "Mesh/CTable.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Mesh::LagrangeP1;

//////////////////////////////////////////////////////////////////////////////

struct MatrixInterpolationFixture
{

    typedef Triag2D ETYPE;

    const static Uint NDOF = ETYPE::nb_nodes;
    const static Uint nbQdPts = 4;
    const static RealVector4 xi_q;
    const static RealVector4 eta_q;
    const static RealVector4 w;

  /// common setup for each test case
    MatrixInterpolationFixture()
    {
        CTable<Real>::Ptr      V (Common::allocate_component< CTable<Real> >("V"));
        V->set_row_size(NDOF);
        V->resize(nbQdPts);

        CTable<Real>::Ptr  dVdxi (Common::allocate_component< CTable<Real> >("dVdxi"));
        dVdxi->set_row_size(NDOF);
        dVdxi->resize(nbQdPts);

        CTable<Real>::Ptr dVdeta (Common::allocate_component< CTable<Real> >("dVdeta"));
        dVdeta->set_row_size(NDOF);
        dVdeta->resize(nbQdPts);

        ETYPE::CoordsT ref_coord;
        ETYPE::SF::ValueT values;
        ETYPE::SF::GradientT gradients;
        ETYPE::SF::ValueT dSFdxi;
        ETYPE::SF::ValueT dSFdeta;


        // Loop over quadrature points and set the elements of
        // the generalized Vandermonde matrix and of the derivatives of
        // the Vandermonde matrix
        for(Uint iq=0;iq<nbQdPts;++iq) {
     ref_coord[XX] =  xi_q[iq];
           ref_coord[YY] = eta_q[iq];

           ETYPE::SF::compute_value(ref_coord,values);
           V->set_row<RealVector>(iq,values);

		 ETYPE::SF::compute_gradient(ref_coord,gradients);
		 for(Uint i = 0; i<NDOF; ++i) {
		dSFdxi[i]  = gradients(XX,i);
		dSFdeta[i] = gradients(YY,i);
		 }

           dVdxi->set_row<RealVector>(iq,dSFdxi);
           dVdeta->set_row<RealVector>(iq,dSFdeta);

        }

  /*
  //Some printouts:
        //CFinfo << "Finished setting up the Vandermonde matrix" << CFendl;
        for(Uint i=0; i<nbQdPts; ++i) {
    CTable<Real>::ConstRow row = (*V)[i];
          for(Uint j=0; j<NDOF; ++j) {
            CFinfo << row[j] << " ";
    }
          CFinfo << CFendl;
        }
        CFinfo << CFendl;

  //Print the derivatives with respect to xi:
        for(Uint i=0; i<nbQdPts; ++i) {
    CTable<Real>::ConstRow row = (*dVdxi)[i];
          for(Uint j=0; j<NDOF; ++j) {
            CFinfo << row[j] << " ";
    }
          CFinfo << CFendl;
        }
        CFinfo << CFendl;

  //Print the derivatives with respect to eta:
        for(Uint i=0; i<nbQdPts; ++i) {
    CTable<Real>::ConstRow row = (*dVdeta)[i];
          for(Uint j=0; j<NDOF; ++j) {
            CFinfo << row[j] << " ";
    }
          CFinfo << CFendl;
        }
        CFinfo << CFendl;
       */

    } // constructor

  /// common tear-down for each test case
  ~MatrixInterpolationFixture()
  {
  }

};


/// Integration for P1 triangle, 4 quadrature points:
const RealVector4 MatrixInterpolationFixture::xi_q(1.0/3.0, 0.2, 0.6, 0.2);
const RealVector4 MatrixInterpolationFixture::eta_q(1.0/3.0, 0.2, 0.2, 0.6);
const RealVector4 MatrixInterpolationFixture::w(-27.0/96.0, 25.0/96.0, 25.0/96.0, 25.0/96.0);

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MatrixInterpolationSuite, MatrixInterpolationFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( MyInterpolation )
{
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

