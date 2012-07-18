// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_navierstokesmovingreference_Source3D_hpp
#define cf3_sdm_navierstokesmovingreference_Source3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "math/Checks.hpp"

#include "sdm/SourceTerm.hpp"
#include "sdm/navierstokesmovingreference/LibNavierStokesMovingReference.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace navierstokesmovingreference {

////////////////////////////////////////////////////////////////////////////////

class sdm_navierstokes_API Source3D : public SourceTerm< PhysDataBase<5u,3u> >
{
private:
    Real gamma;
    RealVectorNDIM Vtrans;
    RealVector3 Omega;
    RealVector3 a0, dOmegadt;
private:
    RealVector3 r;
    RealVector3 Vr;
    RealVector3 V0;
    RealVector3 Vt;
    RealVector3 at;

    void config_Omega()
    {
        std::vector<Real> Omega_vec= options().value< std::vector<Real> >("Omega");
        cf3_assert(Omega_vec.size() == 3);
//        cf3_assert(Omega_vec[0] == 0);
//        cf3_assert(Omega_vec[1] == 0);
        Omega[XX] = Omega_vec[XX];
        Omega[YY] = Omega_vec[YY];
        Omega[ZZ] = Omega_vec[ZZ];
    }

    void config_Vtrans()
    {
        std::vector<Real> Vtrans_vec= options().value< std::vector<Real> >("Vtrans");
        cf3_assert(Vtrans_vec.size() == 3);
        Vtrans[XX] = Vtrans_vec[XX];
        Vtrans[YY] = Vtrans_vec[YY];
        Vtrans[ZZ] = Vtrans_vec[ZZ];
    }

    void config_a0()
    {
        std::vector<Real> a0_vec= options().value< std::vector<Real> >("a0");
        cf3_assert(a0_vec.size() == 3);
//        cf3_assert(a0_vec[2] == 0);
        a0[XX] = a0_vec[XX];
        a0[YY] = a0_vec[YY];
        a0[ZZ] = a0_vec[ZZ];
    }

    void config_dOmegadt()
    {
        std::vector<Real> dOmegadt_vec= options().value< std::vector<Real> >("dOmegadt");
        cf3_assert(dOmegadt_vec.size() == 3);
//        cf3_assert(dOmegadt_vec[0] == 0);
//        cf3_assert(dOmegadt_vec[1] == 0);
        dOmegadt[XX] = dOmegadt_vec[XX];
        dOmegadt[YY] = dOmegadt_vec[YY];
        dOmegadt[ZZ] = dOmegadt_vec[ZZ];
    }

public:
    static std::string type_name() { return "Source3D"; }

    Source3D(const std::string& name) : SourceTerm< PhysData >(name),
                                            gamma(1.4)
    {

      gamma=1.4;
      Vtrans.setZero();
      Omega.setZero();
      a0.setZero();
      dOmegadt.setZero();
      r.setZero();
      Vr.setZero();
      V0.setZero();
      Vt.setZero();
      at.setZero();

      std::vector<Real> OmegaDefault (3,0), VtransDefault(3,0), a0Default(3,0), dOmegadtDefault(3,0);
      OmegaDefault[XX] = Omega[XX];
      OmegaDefault[YY] = Omega[YY];
      OmegaDefault[ZZ] = Omega[ZZ];

      VtransDefault[XX] = Vtrans[XX];
      VtransDefault[YY] = Vtrans[YY];
      VtransDefault[ZZ] = Vtrans[ZZ];

      a0Default[XX] = a0[XX];
      a0Default[YY] = a0[YY];
      a0Default[ZZ] = a0[ZZ];

      dOmegadtDefault[XX] = dOmegadt[XX];
      dOmegadtDefault[YY] = dOmegadt[YY];
      dOmegadtDefault[ZZ] = dOmegadt[ZZ];

      options().add("Omega", OmegaDefault)
          .description("Rotation vector")
          .mark_basic()
          .attach_trigger(boost::bind( &Source3D::config_Omega, this));

      options().add("Vtrans", VtransDefault)
          .description("Vector of the translation speeds")
          .mark_basic()
          .attach_trigger( boost::bind( &Source3D::config_Vtrans, this));

      options().add("a0", a0Default)
          .description("Acceleration of the translation (DVtrans/Dt)")
          .mark_basic()
          .attach_trigger( boost::bind( &Source3D::config_a0, this));

      options().add("dOmegadt", dOmegadtDefault)
          .description("Acceleration of the rotation")
          .mark_basic()
          .attach_trigger( boost::bind( &Source3D::config_dOmegadt, this));

      options().add("gamma", gamma)
          .description("The heat capacity ratio")
          .link_to(&gamma);
    }

    virtual ~Source3D() {}

    void compute_source(PhysData& data, RealVectorNEQS& source)
    {
        const Real& rho = data.solution[0];

        r[XX]=data.coord[XX];
        r[YY]=data.coord[YY];
        r[ZZ]=data.coord[ZZ];

        Vr[XX] = data.solution[1]/rho;
        Vr[YY] = data.solution[2]/rho;
        Vr[ZZ] = data.solution[3]/rho;

        V0[XX] = Vtrans[XX];
        V0[YY] = Vtrans[YY];
        V0[ZZ] = Vtrans[ZZ];

        Vt = V0 + Omega.cross(r);
        at = a0 + dOmegadt.cross(r) + 2.*(Omega.cross(Vr)) + Omega.cross(Omega.cross(r));

        //        source = RealVectorNEQS::Zero(4);
        //        source.block<2,1>(1,0).noalias() = -rho*at.head(2);
        source[0] = 0.;
        source[1] = -rho*at[XX];
        source[2] = -rho*at[YY];
        source[3] = -rho*at[ZZ];
        source[4] = -rho*(Vr.dot(a0) + (Omega.cross(r)).dot(a0) + V0.dot(at - Omega.cross(Vt)) + Vr.dot(dOmegadt.cross(r)) + (Omega.cross(r)).dot(dOmegadt.cross(r)));

//        try{
//          if (math::Checks::is_nan(source[1])) throw common::BadValue(FromHere(),"");
//          if (math::Checks::is_nan(source[2])) throw common::BadValue(FromHere(),"");
//          if (math::Checks::is_nan(source[3])) throw common::BadValue(FromHere(),"");
//        }
//        catch(...)
//        {
//          std::cout << "Omega = " << Omega.transpose() << std::endl;
//          std::cout << "dOmegadt = " << dOmegadt.transpose() << std::endl;
//          std::cout << "r = " << r.transpose() << std::endl;
//          std::cout << "rho = " << rho << std::endl;
//          std::cout << "Vr = " << Vr.transpose() << std::endl;
//          std::cout << "V0 = " << V0.transpose() << std::endl;
//          std::cout << "a0 = " << a0.transpose() << std::endl;
//          throw common::BadValue(FromHere(),"");
//        }
    }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

////////////////////////////////////////////////////////////////////////////////

} // navierstokesmovingreference
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_navierstokesmovingreference_Source3D_hpp
