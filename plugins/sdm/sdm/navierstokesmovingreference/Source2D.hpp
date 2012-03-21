// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_navierstokesmovingreference_Source2D_hpp
#define cf3_sdm_navierstokesmovingreference_Source2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "sdm/SourceTerm.hpp"
#include "sdm/navierstokesmovingreference/LibNavierStokesMovingReference.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace navierstokesmovingreference {

////////////////////////////////////////////////////////////////////////////////

class sdm_navierstokes_API Source2D : public SourceTerm< PhysDataBase<4u,2u> >
{
private:
    Real gamma;
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

    Source2D(const std::string& name) : SourceTerm< PhysData >(name),
                                            gamma(1.4)
    {
          std::vector<Real> OmegaDefault (3,0), VtransDefault(2,0), a0Default(3,0), dOmegadtDefault(3,0);
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
              .description("Acceleration of the translation (DVtrans/Dt)")
              .mark_basic()
              .attach_trigger( boost::bind( &Source2D::config_a0, this));

          options().add_option("dOmegadt", dOmegadtDefault)
              .description("Acceleration of the rotation")
              .mark_basic()
              .attach_trigger( boost::bind( &Source2D::config_dOmegadt, this));

          options().add_option("gamma", gamma)
              .description("The heat capacity ratio")
              .link_to(&gamma);
    }

    virtual ~Source2D() {}

    void compute_source(PhysData& data, RealVectorNEQS& source)
    {
        RealVector3 r = RealVector3::Zero(3);
        RealVector3 Vr = RealVector3::Zero(3);
        RealVector3 V0 = RealVector3::Zero(3);
        RealVector3 Vt = RealVector3::Zero(3);
        RealVector3 at = RealVector3::Zero(3);

        r.head(2).noalias() = data.coord;
        Vr.head(2).noalias() = data.solution.block<2,1>(1,0)/data.solution[0];
        V0.head(2).noalias() = Vtrans;
        Vt.noalias() = V0 + Omega.cross(r);
        at.noalias() = a0 + dOmegadt.cross(r) + 2*(Omega.cross(Vr)) + Omega.cross(Omega.cross(r));

        source = RealVector4::Zero(4);
        source.block<2,1>(1,0).noalias() = -data.solution[0]*at.head(2);

        source[3] = -data.solution[0]*(Vr.dot(a0) + (Omega.cross(r)).dot(a0) + V0.dot(at - Omega.cross(Vt)) + Vr.dot(dOmegadt.cross(r)) + (Omega.cross(r)).dot(dOmegadt.cross(r)));
    }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

////////////////////////////////////////////////////////////////////////////////

} // navierstokesmovingreferece
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_navierstokesmovingreference_Source2D_hpp
