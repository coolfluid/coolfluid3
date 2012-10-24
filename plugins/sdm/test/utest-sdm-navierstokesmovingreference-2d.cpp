// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the ElementCaches of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::sdm"

#include <iostream>
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

struct Data
{
    RealVector4 solution;
    RealVector2 coord;
    Eigen::Matrix<Real, 4, 2> grad_solution;
};

void flux_diff(Data d1, RealVector2 unit_normal, Real mu_v, RealVector4& F_Dn)
{
    Real mu=1; // moet optie van gemaakt worden
    Real kappa=1; //moet nog een optie van gemaakt worden
    Real R=287.05; // optie van maken
    Real gamma=1.4; // optie van maken

    Real cv = R/(gamma-1);

    RealVector4 f_d, g_d;
    Eigen::Matrix<RealVector4, 2, 1> F_D;

    Real rho = d1.solution[0];
    Real rhou = d1.solution[1];
    Real rhov = d1.solution[2];
    Real rhoE = d1.solution[3];

    Real drho_dx = d1.grad_solution(0,0);
    Real drhou_dx = d1.grad_solution(1,0);
    Real drhov_dx = d1.grad_solution(2,0);
    Real drhoE_dx = d1.grad_solution(3,0);

    Real drho_dy = d1.grad_solution(0,1);
    Real drhou_dy = d1.grad_solution(1,1);
    Real drhov_dy = d1.grad_solution(2,1);
    Real drhoE_dy = d1.grad_solution(3,1);

    Real dT_dx = 1/(rho*cv)* (drhoE_dx - (rhoE/rho-(1/(rho*rho))*(rhou*rhou+rhov*rhov))*drho_dx - (rhou/rho*drhou_dx + rhov/rho*drhov_dx));
    Real dT_dy = 1/(rho*cv)*(drhoE_dy - (rhoE/rho-(1/(rho*rho))*(rhou*rhou+rhov*rhov))*drho_dy - (rhou/rho*drhou_dy+rhov*drhov_dy));

    f_d[0] = 0;
    f_d[1] = 2*mu*(1/rho*drhou_dx-rhou/(rho*rho)*drho_dx) + mu_v*(1/rho*drhou_dx-rhou/(rho*rho)*drho_dx + 1/rho*drhov_dy-rhov/(rho*rho)*drho_dy);
    f_d[2] = mu*(1/rho*drhov_dx-rhov/(rho*rho)*drho_dx+1/rho*drhou_dy-rhou/(rho*rho)*drho_dy);
    f_d[3] = f_d[1]*rhou/rho + f_d[2]*rhov/rho + kappa*dT_dx;

    g_d[0] = 0;
    g_d[1] = mu*(1/rho*drhov_dx-rhov/(rho*rho)*drho_dx+1/rho*drhou_dy-rhou/(rho*rho)*drho_dy);
    g_d[2] = 2*mu*(1/rho*drhov_dy-rhov/(rho*rho)*drho_dy) + mu_v*(1/rho*drhou_dx-rhou/(rho*rho)*drho_dx + 1/rho*drhov_dy-rhov/(rho*rho)*drho_dy);
    g_d[3] = g_d[1]*rhou/rho + g_d[2]*rhov/rho + kappa*dT_dy;

    F_D(0) = f_d;
    F_D(1) = g_d;

    F_Dn = F_D(0)*unit_normal(0) + F_D(1)*unit_normal(1);

}


////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( sdm_MPITests_TestSuite, sdm_MPITests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
//  PE::Comm::instance().init(m_argc,m_argv);
  Core::instance().environment().options().set("log_level", (Uint)INFO);
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
    S0->options().set("Vtrans", Vtransoption);
    S0->options().set("Omega", Omegaoption);

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
    S1->options().set("Vtrans", Vtransoption);
    S1->options().set("Omega", Omegaoption);

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
    S2->options().set("Vtrans", Vtransoption);
    S2->options().set("Omega", Omegaoption);

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
    S3->options().set("Vtrans", Vtransoption);
    S3->options().set("Omega", Omegaoption);
    S3->options().set("a0", a0option);
    S3->options().set("dOmegadt", dOmegadtoption);

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

    C1->options().set("Omega", Omegaoption);
    C1->options().set("Vtrans", Vtransoption);
    C1->options().set("gamma", 1.4);

    C1->compute_analytical_flux(Data, normal, flux, wavespeed);

    BOOST_CHECK_CLOSE(flux[0],      1. , tol);
    BOOST_CHECK_CLOSE(flux[1], 101102.4, tol);
    BOOST_CHECK_CLOSE(flux[2],      0. , tol);
    BOOST_CHECK_CLOSE(flux[3], 353855.4, tol);

//    std::cout << "wavespeed analytical = " << wavespeed << std::endl;

    Omegaoption[0] = 0.;
    Omegaoption[1] = 0.;
    Omegaoption[2] = 10.;

    Vtransoption[0] = 1.;
    Vtransoption[1] = 0.;

    Data.coord << 1., 0.;
    Data.solution << 1., 1., 0, 252754.;

    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Convection2D> C2 = allocate_component<cf3::sdm::navierstokesmovingreference::Convection2D>("convection1");

    C2->options().set("Omega", Omegaoption);
    C2->options().set("Vtrans", Vtransoption);
    C2->options().set("gamma", 1.4);

    wavespeed = 0;
    C2->compute_analytical_flux(Data, normal, flux, wavespeed);

    BOOST_CHECK_CLOSE(flux[0],      1. , tol);
    BOOST_CHECK_CLOSE(flux[1], 101122.6, tol);
    BOOST_CHECK_CLOSE(flux[2],      0. , tol);
    BOOST_CHECK_CLOSE(flux[3], 353875.6, tol);

//    std::cout << "wavespeed numerical = " << wavespeed << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_diffusion )
{
    const Real tol (0.000001);

    Data data1;
    RealVector2 unit_normal;
    unit_normal << 1, 0;
    RealVector4 F_Dn = RealVector4::Zero();

    Real mu_v=0; // moet nog optie van gemaakt worden

    data1.solution << 1., 1., 0., 100.;
    data1.coord << 1., 0;
    data1.grad_solution << 1., 0.,
                           1., 0.,
                           0., 0.,
                           0., 0.;

    flux_diff(data1, unit_normal, mu_v, F_Dn);


    BOOST_CHECK_CLOSE(F_Dn[0], 0., tol);
    BOOST_CHECK_CLOSE(F_Dn[1], 0., tol);
    BOOST_CHECK_CLOSE(F_Dn[2], 0., tol);
    BOOST_CHECK_CLOSE(F_Dn[3], -0.139348546, tol);

////////////////////////////////////////////////////////////////////

    //mu_v different from zero en drhou_dx verschillend van drho_dx
    unit_normal << 1, 0;

    mu_v=1.; // moet nog optie van gemaakt worden

    data1.solution << 1., 1., 0., 100.;
    data1.coord << 1., 0;
    data1.grad_solution << 1., 0.,
                           2., 0.,
                           0., 0.,
                           0., 0.;

    F_Dn = RealVector4::Zero();
    flux_diff(data1, unit_normal, mu_v, F_Dn);

    BOOST_CHECK_CLOSE(F_Dn[0], 0., tol);
    BOOST_CHECK_CLOSE(F_Dn[1], 3., tol);
    BOOST_CHECK_CLOSE(F_Dn[2], 0., tol);
    BOOST_CHECK_CLOSE(F_Dn[3], 2.859257969, tol);

// ////////////////////////////////////////////////////////

    unit_normal << 0., 1.;

    mu_v=1.; // moet nog optie van gemaakt worden

    data1.solution << 1., 0., 1., 100.;
    data1.coord << 1., 0.;
    data1.grad_solution << 0., 1.,
                           0., 0.,
                           0., 2.,
                           0., 0.;

    F_Dn = RealVector4::Zero();
    flux_diff(data1, unit_normal, mu_v, F_Dn);

    BOOST_CHECK_CLOSE(F_Dn[0], 0., tol);
    BOOST_CHECK_CLOSE(F_Dn[1], 0., tol);
    BOOST_CHECK_CLOSE(F_Dn[2], 3., tol);
    BOOST_CHECK_CLOSE(F_Dn[3], 2.859257969, tol);

///////////////////////////////////////////////////////////
    unit_normal << 1, 0;

    mu_v=1.; // moet nog optie van gemaakt worden

    data1.solution << 1., 1., 1., 100.;
    data1.coord << 1., 0;
    data1.grad_solution << 1., 1.,
                           2., 2.,
                           2., 2.,
                           1., 0.;

    F_Dn = RealVector4::Zero();
    flux_diff(data1, unit_normal, mu_v, F_Dn);

    BOOST_CHECK_CLOSE(F_Dn[0], 0., tol);
    BOOST_CHECK_CLOSE(F_Dn[1], 4., tol);
    BOOST_CHECK_CLOSE(F_Dn[2], 2., tol);
    BOOST_CHECK_CLOSE(F_Dn[3], 5.859257969, tol);
///////////////////////////////////////////////////////

    unit_normal << 0, 1;

    mu_v=1.; // moet nog optie van gemaakt worden


    data1.solution << 1., 1., 1., 100.;
    data1.coord << 1., 0;
    data1.grad_solution << 1., 1.,
                           2., 2.,
                           2., 2.,
                           0., 1.;

    F_Dn = RealVector4::Zero();
    flux_diff(data1, unit_normal, mu_v, F_Dn);

    BOOST_CHECK_CLOSE(F_Dn[0], 0., tol);
    BOOST_CHECK_CLOSE(F_Dn[1], 2., tol);
    BOOST_CHECK_CLOSE(F_Dn[2], 4., tol);
    BOOST_CHECK_CLOSE(F_Dn[3], 5.859257969, tol);
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
//  PE::Comm::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
