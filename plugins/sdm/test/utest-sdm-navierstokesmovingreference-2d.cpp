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

namespace cf3{
namespace sdm {
namespace navierstokesmovingreference {

////////////////////////////////////////////////////////////////////////////////
struct Datatype
{
    RealVector4 solution;
    RealVector2 coordinates;
};

class  Source2D : public common::Component
{
private:
    RealVector2 Vtrans;
    RealVector3 Omega;
    RealVector3 a0, dOmegadt;

    void config_Omega()
    {
        std::vector<Real> Omega_vec= options().option("Omega").value< std::vector<Real> >();
        cf3_assert(Omega_vec.size() == 3);
        cf3_assert(Omega_vec[0] == 0);
        cf3_assert(Omega_vec[1] == 0);
        Omega[0] = Omega_vec[0];
        Omega[1] = Omega_vec[1];
        Omega[2] = Omega_vec[2];
    }

    void config_Vtrans()
    {
        std::vector<Real> Vtrans_vec= options().option("Vtrans").value< std::vector<Real> >();
        cf3_assert(Vtrans_vec.size() == 2);
        Vtrans[0] = Vtrans_vec[0];
        Vtrans[1] = Vtrans_vec[1];
    }

    void config_a0()
    {
        std::vector<Real> a0_vec= options().option("a0").value< std::vector<Real> >();
        cf3_assert(a0_vec.size() == 3);
        cf3_assert(a0_vec[2] == 0);
        a0[0] = a0_vec[0];
        a0[1] = a0_vec[1];
        a0[2] = a0_vec[2];
    }

    void config_dOmegadt()
    {
        std::vector<Real> dOmegadt_vec= options().option("dOmegadt").value< std::vector<Real> >();
        cf3_assert(dOmegadt_vec.size() == 3);
        cf3_assert(dOmegadt_vec[0] == 0);
        cf3_assert(dOmegadt_vec[1] == 0);
        dOmegadt[0] = dOmegadt_vec[0];
        dOmegadt[1] = dOmegadt_vec[1];
        dOmegadt[2] = dOmegadt_vec[2];
    }

public:
  
    static std::string type_name() { return "Source2D"; }
    Source2D(const std::string& name) : common::Component(name)
    {
        std::vector<Real> OmegaDefault(3,0), VtransDefault(2,0), a0Default(3,0), dOmegadtDefault(3,0);
        Omega    << 0., 0., 0.;
        Vtrans   << 0., 0.;
        a0       << 0., 0., 0.;
        dOmegadt << 0., 0., 0.;

        OmegaDefault[0] = Omega[0];
        OmegaDefault[1] = Omega[1];
        OmegaDefault[2] = Omega[2];

        VtransDefault[0] = Vtrans[0];
        VtransDefault[1] = Vtrans[1];

        a0Default[0] = a0[0];
        a0Default[1] = a0[1];
        a0Default[2] = a0[2];

        dOmegadtDefault[0] = dOmegadt[0];
        dOmegadtDefault[1] = dOmegadt[1];
        dOmegadtDefault[2] = dOmegadt[2];

        options().add_option("Omega", OmegaDefault)
            .description("Rotation vector")
            .mark_basic()
            .attach_trigger(boost::bind( &Source2D::config_Omega, this));

        options().add_option("Vtrans", VtransDefault)
            .description("Vector of the translation speeds")
            .mark_basic()
            .attach_trigger( boost::bind( &Source2D::config_Vtrans, this));

        options().add_option("a0", a0Default)
            .description("Total time derivative (in absolute reference frame)")
            .mark_basic()
            .attach_trigger( boost::bind( &Source2D::config_a0, this));

        options().add_option("dOmegadt", dOmegadtDefault)
            .description("Time derivative of rotation vector")
            .mark_basic()
            .attach_trigger( boost::bind( &Source2D::config_dOmegadt, this));
    }
    virtual ~Source2D() {}

    void compute_source(const navierstokesmovingreference::Datatype& data, RealVector4& source)
    {
        RealVector3 r = RealVector3::Zero(3);
        RealVector3 Vr = RealVector3::Zero(3);
        RealVector3 V0 = RealVector3::Zero(3);
        RealVector3 Vt = RealVector3::Zero(3);
        RealVector3 at = RealVector3::Zero(3);

        r.head(2).noalias() = data.coordinates;
        Vr.head(2).noalias() = data.solution.block<2,1>(1,0)/data.solution[0];
        V0.head(2).noalias() = Vtrans;
        Vt.noalias() = V0 + Omega.cross(r);
        at.noalias() = a0 + dOmegadt.cross(r) + 2*(Omega.cross(Vr)) + Omega.cross(Omega.cross(r));

        source = RealVector4::Zero(4);
        source.block<2,1>(1,0).noalias() = -data.solution[0]*at.head(2);

        source[3] = -data.solution[0]*(Vr.dot(a0) + (Omega.cross(r)).dot(a0) + V0.dot(at - Omega.cross(Vt)) + Vr.dot(dOmegadt.cross(r)) + (Omega.cross(r)).dot(dOmegadt.cross(r)));
    }
};

////////////////////////////////////////////////////////////////////////////////
}
}
}

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
    navierstokesmovingreference::Datatype d0;
    RealVector4 s0;

    std::vector<Real> Vtransoption(2,0), Omegaoption(3,0), a0option(3,0), dOmegadtoption(3,0);

    // Test without relative movement

    d0.solution << 1., 1., 0., 252754.; //P = 101101.4
    d0.coordinates << 1., 1.;

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
    navierstokesmovingreference::Datatype d1;
    RealVector4 s1;

    d1.solution << 1., 2., 0., 252754.; //P = 101101.4
    d1.coordinates << 1., 1.;

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
    navierstokesmovingreference::Datatype d2;
    RealVector4 s2;

    d2.solution << 1., 2., 2., 252754.; //P = 101101.4
    d2.coordinates << 1., 1.;

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
    navierstokesmovingreference::Datatype d3;
    RealVector4 s3;

    d3.solution << 1., 2., 2., 252754.; //P = 101101.4
    d3.coordinates << 1., 1.;

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
    Real wavespeed;

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

    C2->compute_analytical_flux(Data, normal, flux, wavespeed);

    BOOST_CHECK_CLOSE(flux[0],      1. , tol);
    BOOST_CHECK_CLOSE(flux[1], 101122.6, tol);
    BOOST_CHECK_CLOSE(flux[2],      0. , tol);
    BOOST_CHECK_CLOSE(flux[3], 353875.6, tol);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
//  PE::Comm::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
