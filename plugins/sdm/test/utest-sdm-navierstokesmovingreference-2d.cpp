// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the ElementCaches of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::sdm"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/List.hpp"

#include "common/PE/Comm.hpp"

#include "math/Consts.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"

#include "sdm/navierstokesmovingreference/Convection2D.hpp"
#include "sdm/navierstokesmovingreference/Source2D.hpp"

using namespace cf3;
using namespace cf3::math;
using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::solver;
using namespace cf3::sdm;

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


////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( sdm_MPITests_TestSuite, sdm_MPITests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
//  PE::Comm::instance().init(m_argc,m_argv);
  Core::instance().environment().options().configure_option("log_level", (Uint)INFO);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_source )
{
    const Real tol (0.000001);
    PhysDataBase<4, 2> d0;
    RealVector4 s0;

    std::vector<Real> Vtransoption(2,0), Omegaoption(3,0), a0option(3,0), dOmegadtoption(3,0);

    // Test without relative movement

    d0.solution << 1., 1., 0., 252754.; //P = 101101.4
    d0.coord << 1., 1.;

    Vtransoption[0] = 0.;
    Vtransoption[1] = 0.;

    Omegaoption[0] = 0.;
    Omegaoption[1] = 0.;
    Omegaoption[2] = 0.;

    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Source2D> S0 = allocate_component<cf3::sdm::navierstokesmovingreference::Source2D>("source0");
    S0->options().configure_option("Vtrans", Vtransoption);
    S0->options().configure_option("Omega", Omegaoption);

    S0->compute_source(d0, s0);

    BOOST_CHECK_CLOSE(s0[0], 0., tol);
    BOOST_CHECK_CLOSE(s0[1], 0., tol);
    BOOST_CHECK_CLOSE(s0[2], 0., tol);
    BOOST_CHECK_CLOSE(s0[3], 0., tol);


    // Test with relative movement, u only velocity component, no acceleration of the relative reference frame
    PhysDataBase<4, 2> d1;
    RealVector4 s1;

    d1.solution << 1., 2., 0., 252754.; //P = 101101.4
    d1.coord << 1., 1.;

    Vtransoption[0] = 1.;
    Vtransoption[1] = 1.;

    Omegaoption[0] = 0.;
    Omegaoption[1] = 0.;
    Omegaoption[2] = 10;

    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Source2D> S1 = allocate_component<cf3::sdm::navierstokesmovingreference::Source2D>("source1");
    S1->options().configure_option("Vtrans", Vtransoption);
    S1->options().configure_option("Omega", Omegaoption);

    S1->compute_source(d1, s1);

    BOOST_CHECK_CLOSE(s1[0],   0., tol);
    BOOST_CHECK_CLOSE(s1[1], 100., tol);
    BOOST_CHECK_CLOSE(s1[2],  60., tol);
    BOOST_CHECK_CLOSE(s1[3], -40., tol);

    // Test with relavite movement, u and v different from zero, no acceleration of the relative reference frame
    PhysDataBase<4, 2> d2;
    RealVector4 s2;

    d2.solution << 1., 2., 2., 252754.; //P = 101101.4
    d2.coord << 1., 1.;

    Vtransoption[0] = 1.;
    Vtransoption[1] = 1.;

    Omegaoption[0] = 0.;
    Omegaoption[1] = 0.;
    Omegaoption[2] = 10;

    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Source2D> S2 = allocate_component<cf3::sdm::navierstokesmovingreference::Source2D>("source2");
    S2->options().configure_option("Vtrans", Vtransoption);
    S2->options().configure_option("Omega", Omegaoption);

    S2->compute_source(d2, s2);

    BOOST_CHECK_CLOSE(s2[0],   0., tol);
    BOOST_CHECK_CLOSE(s2[1], 140., tol);
    BOOST_CHECK_CLOSE(s2[2],  60., tol);
    BOOST_CHECK_CLOSE(s2[3],   0., tol);

    // Test without initial relavite velocity, u and v zero, acceleration of the relative reference frame
    PhysDataBase<4, 2> d3;
    RealVector4 s3;

    d3.solution << 1., 2., 2., 252754.; //P = 101101.4
    d3.coord << 1., 1.;

    Vtransoption[0] = 0.;
    Vtransoption[1] = 0.;

    Omegaoption[0] = 0.;
    Omegaoption[1] = 0.;
    Omegaoption[2] = 0.;

    a0option[0] = 1.;
    a0option[1] = 1.;
    a0option[2] = 0.;

    dOmegadtoption[0] = 0.;
    dOmegadtoption[1] = 0.;
    dOmegadtoption[2] = 1.;

    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Source2D> S3 = allocate_component<cf3::sdm::navierstokesmovingreference::Source2D>("source3");
    S3->options().configure_option("Vtrans", Vtransoption);
    S3->options().configure_option("Omega", Omegaoption);
    S3->options().configure_option("a0", a0option);
    S3->options().configure_option("dOmegadt", dOmegadtoption);

    S3->compute_source(d3, s3);

    BOOST_CHECK_CLOSE(s3[0],  0., tol);
    BOOST_CHECK_CLOSE(s3[1],  0., tol);
    BOOST_CHECK_CLOSE(s3[2], -2., tol);
    BOOST_CHECK_CLOSE(s3[3], -4., tol);
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( test_convection )
{
    const Real tol (0.000001);

    RealVector2 normal(1, 0);
    RealVector4 flux;
    Real wavespeed = 0;

    PhysDataBase<4, 2> Data;

    std::vector<Real> Vtransoption(2,0), Omegaoption(3,0);

    Omegaoption[0] = 0.;
    Omegaoption[1] = 0.;
    Omegaoption[2] = 0.;

    Vtransoption[0] = 0.;
    Vtransoption[1] = 0.;

    Data.coord << 1., 0.;
    Data.solution << 1., 1., 0, 252754.;

    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Convection2D> C1 = allocate_component<cf3::sdm::navierstokesmovingreference::Convection2D>("convection1");

    C1->options().configure_option("Omega", Omegaoption);
    C1->options().configure_option("Vtrans", Vtransoption);
    C1->options().configure_option("gamma", 1.4);

    C1->compute_analytical_flux(Data, normal, flux, wavespeed);

    BOOST_CHECK_CLOSE(flux[0],      1. , tol);
    BOOST_CHECK_CLOSE(flux[1], 101102.4, tol);
    BOOST_CHECK_CLOSE(flux[2],      0. , tol);
    BOOST_CHECK_CLOSE(flux[3], 353855.4, tol);

    std::cout << "wavespeed analytical = " << wavespeed << std::endl;

    Omegaoption[0] = 0.;
    Omegaoption[1] = 0.;
    Omegaoption[2] = 10.;

    Vtransoption[0] = 1.;
    Vtransoption[1] = 0.;

    Data.coord << 1., 0.;
    Data.solution << 1., 1., 0, 252754.;

    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Convection2D> C2 = allocate_component<cf3::sdm::navierstokesmovingreference::Convection2D>("convection1");

    C2->options().configure_option("Omega", Omegaoption);
    C2->options().configure_option("Vtrans", Vtransoption);
    C2->options().configure_option("gamma", 1.4);

    wavespeed = 0;
    C2->compute_analytical_flux(Data, normal, flux, wavespeed);

    BOOST_CHECK_CLOSE(flux[0],      1. , tol);
    BOOST_CHECK_CLOSE(flux[1], 101122.6, tol);
    BOOST_CHECK_CLOSE(flux[2],      0. , tol);
    BOOST_CHECK_CLOSE(flux[3], 353875.6, tol);

    std::cout << "wavespeed numerical = " << wavespeed << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
//  PE::Comm::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
