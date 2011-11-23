// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::SFDM transformations"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"
#include "common/StringConversion.hpp"

#include "math/Consts.hpp"
#include "math/MatrixTypes.hpp"
#include "math/Defs.hpp"

#include "mesh/GeoShape.hpp"

#include "mesh/LagrangeP1/Quad2D.hpp"

#include "SFDM/Tags.hpp"

#include "SFDM/LagrangeLocally1D.hpp"


using namespace boost::assign;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::SFDM;

//////////////////////////////////////////////////////////////////////////////

#define CF3_CHECK_EQUAL(x1,x2)\
do {\
  Real fraction=100*math::Consts::eps(); \
  if (x2 == 0) \
    BOOST_CHECK_SMALL(x1,fraction); \
  else if (x1 == 0) \
      BOOST_CHECK_SMALL(x2,fraction); \
  else \
    BOOST_CHECK_CLOSE_FRACTION(x1,x2,fraction);\
} while(false)

//////////////////////////////////////////////////////////////////////////////

struct SFDM_MPITests_Fixture
{
  /// common setup for each test case
  SFDM_MPITests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~SFDM_MPITests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( SFDM_solver_TestSuite, SFDM_MPITests_Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
#ifdef test_is_mpi
  PE::Comm::instance().init(m_argc,m_argv);
#endif
  Core::instance().environment().configure_option("log_level",(Uint)INFO);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( jacobian_eq_half_rot )
{
  // Rotation matrix, counterclockwise with angle
  Real angle = math::Consts::pi()/4.;
  RealMatrix R(2,2);
  R << std::cos(angle) ,  -std::sin(angle),
       std::sin(angle)  ,  std::cos(angle);
  Real scaling = 2.; // 2 times larger than mapped cell

  // Create geometry nodes as rotated coordinates
  RealMatrix geometry_nodes = (R * scaling*(RealMatrix(4,2) << 0.,0.  , 2.,0.  ,  2.,2.  ,  0.,2. ).finished().transpose()).transpose();
  CFinfo << "geometry_nodes = \n" << geometry_nodes << CFendl;

  // Cell in mapped coordinates with a coordinate p
  RealVector mapped_p = (RealVector(2) << 1.,0.).finished();
  RealRowVector mapped_vec = (RealVector(2) << 1.,0.).finished() / scaling;

  //   3--------------2
  //   |              |
  //   |              |
  //   |              p-----> (2,0) mapped_vec
  //   |              |
  //   |              |
  //   0--------------1

  RealMatrix jacobian = mesh::LagrangeP1::Quad2D::jacobian(mapped_p,geometry_nodes);
  CFinfo << "jacobian = \n" << jacobian << CFendl;
  CFinfo << "determinant = " << jacobian.determinant() << CFendl;
  RealMatrix T = jacobian.determinant() * jacobian.inverse();
  CFinfo << "T = \n" << T << CFendl;
  CFinfo << "T.determinant = " << T.determinant() << CFendl;
  CFinfo << "Tinv = \n" << T.inverse() << CFendl;
  CFinfo << "J/|J| = \n" << jacobian / jacobian.determinant() << CFendl;

  CFinfo << "plane_jacobian_normal[KSI] = " << mesh::LagrangeP1::Quad2D::plane_jacobian_normal(mapped_p,geometry_nodes,KSI).transpose() << CFendl;
  CFinfo << "plane_jacobian_normal[ETA] = " << mesh::LagrangeP1::Quad2D::plane_jacobian_normal(mapped_p,geometry_nodes,ETA).transpose() << CFendl;

  // Transform vector from mapped space to physical space
  RealRowVector phys_vec(2);
  phys_vec = mapped_vec*T.inverse();
  CFinfo << "phys_vec = " << phys_vec.transpose() << CFendl;
  CFinfo << "phys_vec.norm() = " << phys_vec.norm() << CFendl;

  // Transform vector from physical space to mapped space
  mapped_vec = phys_vec*T;
  CFinfo << "local_vec = " << mapped_vec << CFendl;
  CFinfo << "local_vec.norm() = " << mapped_vec.norm() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
#ifdef test_is_mpi
  PE::Comm::instance().finalize();
#endif
}

///////////////////////////////////////////////////////////////////////////////



BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
