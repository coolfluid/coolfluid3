// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::sdm"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/OptionList.hpp"
#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"
#include "common/StringConversion.hpp"

#include "math/Consts.hpp"
#include "math/MatrixTypes.hpp"
#include "math/Defs.hpp"

#include "mesh/GeoShape.hpp"

#include "sdm/Tags.hpp"

#include "sdm/LagrangeLocally1D.hpp"


using namespace boost::assign;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::sdm;

//////////////////////////////////////////////////////////////////////////////

#define CF3_CHECK_EQUAL(x1,x2)\
do {\
  Real fraction=200*math::Consts::eps(); \
  if (x2 == 0) \
    BOOST_CHECK_SMALL(x1,fraction); \
  else if (x1 == 0) \
      BOOST_CHECK_SMALL(x2,fraction); \
  else \
    BOOST_CHECK_CLOSE_FRACTION(x1,x2,fraction);\
} while(false)

//////////////////////////////////////////////////////////////////////////////

struct sdm_MPITests_Fixture
{
  /// common setup for each test case
  sdm_MPITests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~sdm_MPITests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

//////////////////////////////////////////////////////////////////////////////

template <typename SF>
void test_convection(const SF& sf)
{
  for (Uint i=0; i<sf.nb_faces(); ++i)
    CFdebug << "face_flx_pts("<<i<<") = " << to_str(sf.face_flx_pts(i)) << CFendl;

  RealMatrix solution = sf.sol_pts();
  RealMatrix residual = solution; residual.setZero();
  RealVector wave_speed(sf.nb_sol_pts()); wave_speed.setZero();

  CFdebug << "solution = \n" << solution << CFendl << CFendl;

  Uint count_interpolate_sol_to_flx(0);
  Uint count_interpolate_flx_to_sol(0);
  Uint count_interpolate_grad_flx_to_sol(0);
  Real count_riemann_problems(0);
  Uint count_flux_eval(0);

  boost_foreach(const Uint flx_pt, sf.interior_flx_pts())
  {
    CFdebug << "flx_pt = " << flx_pt << CFendl;

    CFdebug << " - interpolate" << CFendl;
    RealVector sol_in_flx_pt(solution.cols()); sol_in_flx_pt.setZero();
    boost_foreach(Uint sol_pt, sf.interpolate_sol_to_flx_used_sol_pts(flx_pt))
    {
      ++count_interpolate_sol_to_flx;
      Real coeff = sf.interpolate_sol_to_flx_coeff(flx_pt,sol_pt);
      cf3_assert(coeff != 0); // we made sure of this, that this sol_pt would not be used
      CFdebug << "    sol_pt = " << sol_pt;
      CFdebug << "    coeff = " << coeff << CFendl;
      sol_in_flx_pt += coeff * solution.row(sol_pt);
    }

    ++count_flux_eval;
    RealVector flux = sol_in_flx_pt*1.;
    CFdebug << " - flux = " << flux.transpose() << CFendl;

    Real ws = 1.;
    CFdebug << " - wave_speed = " << ws << CFendl;

    boost_foreach(const Uint dir, sf.flx_pt_dirs(flx_pt))
    {
      CFdebug << " - derivative flux to " << dir << CFendl;
      boost_foreach(Uint sol_pt, sf.interpolate_grad_flx_to_sol_used_sol_pts(flx_pt,dir) )
      {
        ++count_interpolate_grad_flx_to_sol;
        Real coeff = sf.interpolate_grad_flx_to_sol_coeff(flx_pt,dir,sol_pt);
        cf3_assert(coeff != 0); // we made sure of this, that this sol_pt would not be used
        CFdebug << "    sol_pt = " << sol_pt;
        CFdebug << "    coeff = " << coeff << CFendl;
        for (Uint v=0; v<solution.cols(); ++v)
        {
          CFdebug << "    add " << -coeff * flux[v] << CFendl;
          residual(sol_pt,v) -= coeff * flux[v];
        }
      }
      CFdebug << " - interpolate wavespeed in dir " << dir << CFendl;
      boost_foreach(Uint sol_pt, sf.interpolate_flx_to_sol_used_sol_pts(flx_pt,dir) )
      {
        ++count_interpolate_flx_to_sol;
        Real coeff = sf.interpolate_flx_to_sol_coeff(flx_pt,dir,sol_pt);
        cf3_assert(coeff != 0); // we made sure of this, that this sol_pt would not be used
        CFdebug << "    sol_pt = " << sol_pt;
        CFdebug << "    coeff = " << coeff << CFendl;
        wave_speed[sol_pt] += coeff * ws;
      }
    }
  }

  for (Uint face=0; face<sf.nb_faces(); ++face) {
    boost_foreach(const Uint flx_pt, sf.face_flx_pts(face))
    {
      CFdebug << "flx_pt = " << flx_pt << CFendl;

      CFdebug << " - interpolate" << CFendl;
      RealVector sol_in_flx_pt(solution.cols()); sol_in_flx_pt.setZero();
      boost_foreach(Uint sol_pt, sf.interpolate_sol_to_flx_used_sol_pts(flx_pt))
      {
        ++count_interpolate_sol_to_flx;
        Real coeff = sf.interpolate_sol_to_flx_coeff(flx_pt,sol_pt);
        cf3_assert(coeff != 0); // we made sure of this, that this sol_pt would not be used
        CFdebug << "    sol_pt = " << sol_pt;
        CFdebug << "    coeff = " << coeff << CFendl;
        sol_in_flx_pt += coeff * solution.row(sol_pt);
      }

      // divide by faces, as this part is only done once between different cells
      count_riemann_problems += (Real)sf.dimensionality()/(Real)sf.nb_faces();
      RealVector flux = sol_in_flx_pt*1.;
      CFdebug << " - flux = " << flux.transpose() << CFendl;

      Real ws = 1.;
      CFdebug << " - wave_speed = " << ws << CFendl;

      BOOST_CHECK_EQUAL(sf.face_normals().cols(),sf.dimensionality());

      boost_foreach(const Uint dir, sf.flx_pt_dirs(flx_pt))
      {
        CFdebug << " - derivative flux to " << dir << CFendl;
        boost_foreach(Uint sol_pt, sf.interpolate_grad_flx_to_sol_used_sol_pts(flx_pt,dir) )
        {
          BOOST_CHECK_EQUAL(std::abs(sf.face_normals()(face,dir)),1.);

          ++count_interpolate_grad_flx_to_sol;
          Real coeff = sf.interpolate_grad_flx_to_sol_coeff(flx_pt,dir,sol_pt);
          cf3_assert(coeff != 0); // we made sure of this, that this sol_pt would not be used
          CFdebug << "    sol_pt = " << sol_pt;
          CFdebug << "    coeff = " << coeff << CFendl;
          for (Uint v=0; v<solution.cols(); ++v)
          {
            CFdebug << "    add " << -coeff * flux[v] << CFendl;
            residual(sol_pt,v) -= coeff * flux[v];
          }
        }
        CFdebug << " - interpolate wavespeed in dir " << dir << CFendl;
        boost_foreach(Uint sol_pt, sf.interpolate_flx_to_sol_used_sol_pts(flx_pt,dir) )
        {
          ++count_interpolate_flx_to_sol;
          Real coeff = sf.interpolate_flx_to_sol_coeff(flx_pt,dir,sol_pt);
          cf3_assert(coeff != 0); // we made sure of this, that this sol_pt would not be used
          CFdebug << "    sol_pt = " << sol_pt;
          CFdebug << "    coeff = " << coeff << CFendl;
          wave_speed[sol_pt] += coeff * ws;
        }

      }
    }
  }

  CFdebug << "residual = \n" << residual << CFendl;

  for (Uint i=0; i<sf.nb_sol_pts(); ++i) {
    for (Uint j=0; j<residual.cols(); ++j) {
      if (sf.nb_sol_pts()==1)
        CF3_CHECK_EQUAL(residual(i,j),0.);
      else
        CF3_CHECK_EQUAL(residual(i,j),-1.);
    }
    CF3_CHECK_EQUAL(wave_speed[i],(Real)sf.dimensionality());
  }

  Real DOF = sf.nb_sol_pts();
  CFinfo << "statistics "<<sf.shape()<<"-P"<<sf.order()<<":\n";
  CFinfo << "-------------------\n";
  CFinfo << "count_interpolate_sol_to_flx      = " << count_interpolate_sol_to_flx << "\t   (" << count_interpolate_sol_to_flx/DOF <<")" << CFendl;
  CFinfo << "count_interpolate_flx_to_sol      = " << count_interpolate_flx_to_sol << "\t   (" << count_interpolate_flx_to_sol/DOF <<")" << CFendl;
  CFinfo << "count_interpolate_grad_flx_to_sol = " << count_interpolate_grad_flx_to_sol << "\t   (" << count_interpolate_grad_flx_to_sol/DOF <<")" << CFendl;
  CFinfo << "count_flux_eval                   = " << count_flux_eval << "\t   (" << count_flux_eval/DOF <<")" << CFendl;
  CFinfo << "count_riemann_problems            = " << count_riemann_problems << "\t   (" << count_riemann_problems/DOF <<")" << CFendl;
  CFinfo << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( sdm_solver_TestSuite, sdm_MPITests_Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
#ifdef test_is_mpi
  PE::Comm::instance().init(m_argc,m_argv);
#endif
  Core::instance().environment().options().set("log_level",(Uint)INFO);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_P0_line )
{
  test_convection( *allocate_component< LineLagrange1D<0> >("sf") );
}

BOOST_AUTO_TEST_CASE( test_P1_line )
{
  test_convection( *allocate_component< LineLagrange1D<1> >("sf") );
}

BOOST_AUTO_TEST_CASE( test_P2_line )
{
  test_convection( *allocate_component< LineLagrange1D<2> >("sf") );
}

BOOST_AUTO_TEST_CASE( test_P3_line )
{
  test_convection( *allocate_component< LineLagrange1D<3> >("sf") );
}

BOOST_AUTO_TEST_CASE( test_P4_line )
{
  test_convection( *allocate_component< LineLagrange1D<4> >("sf") );
}

BOOST_AUTO_TEST_CASE( test_P5_line )
{
  test_convection( *allocate_component< LineLagrange1D<5> >("sf") );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_P0_quad )
{
  test_convection( *allocate_component< QuadLagrange1D<0> >("sf") );
}

BOOST_AUTO_TEST_CASE( test_P1_quad )
{
  test_convection( *allocate_component< QuadLagrange1D<1> >("sf") );
}

BOOST_AUTO_TEST_CASE( test_P2_quad )
{
  test_convection( *allocate_component< QuadLagrange1D<2> >("sf") );
}

BOOST_AUTO_TEST_CASE( test_P3_quad )
{
  test_convection( *allocate_component< QuadLagrange1D<3> >("sf") );
}

BOOST_AUTO_TEST_CASE( test_P4_quad )
{
  test_convection( *allocate_component< QuadLagrange1D<4> >("sf") );
}

BOOST_AUTO_TEST_CASE( test_P5_quad )
{
  test_convection( *allocate_component< QuadLagrange1D<5> >("sf") );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_P0_hexa )
{
  test_convection( *allocate_component< HexaLagrange1D<0> >("sf") );
}

BOOST_AUTO_TEST_CASE( test_P1_hexa )
{
  test_convection( *allocate_component< HexaLagrange1D<1> >("sf") );
}

BOOST_AUTO_TEST_CASE( test_P2_hexa )
{
  test_convection( *allocate_component< HexaLagrange1D<2> >("sf") );
}

BOOST_AUTO_TEST_CASE( test_P3_hexa )
{
  test_convection( *allocate_component< HexaLagrange1D<3> >("sf") );
}

BOOST_AUTO_TEST_CASE( test_P4_hexa )
{
  test_convection( *allocate_component< HexaLagrange1D<4> >("sf") );
}

BOOST_AUTO_TEST_CASE( test_P5_hexa )
{
  test_convection( *allocate_component< HexaLagrange1D<5> >("sf") );
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
