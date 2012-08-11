// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_BCSubsonicOutlet2D_hpp
#define cf3_sdm_lineuler_BCSubsonicOutlet2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "sdm/BCWeak.hpp"
#include "sdm/lineuler/LibLinEuler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

////////////////////////////////////////////////////////////////////////////////

class sdm_lineuler_API BCSubsonicOutlet2D : public BCWeak< PhysDataBase<4u,2u> >
{
public:
  static std::string type_name() { return "BCSubsonicOutlet2D"; }
  BCSubsonicOutlet2D(const std::string& name) : BCWeak< PhysData >(name)
  {
    m_c0 = 1.;
//    m_u0n = 0.5;
//    m_u0s = 0.;
    std::vector<Real> U0(NDIM);
    U0[XX] = 0.5;
    U0[YY] = 0.;
    options().add("U0",U0).mark_basic();
    options().add("c0",m_c0).mark_basic();
    options().add("alpha",0.);
  }
  virtual ~BCSubsonicOutlet2D() {}

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:

  virtual void initialize()
  {
    BCWeak< PhysData >::initialize();
    flx_pt_plane_jacobian_normal = shared_caches().get_cache< FluxPointPlaneJacobianNormal<NDIM> >();
    flx_pt_plane_jacobian_normal->options().set("space",solution_field().dict().handle<mesh::Dictionary>());

//    m_u0n = options().value< std::vector<Real> >("U0")[XX];
//    m_u0s = options().value< std::vector<Real> >("U0")[YY];
    m_c0  = options().value< Real >("c0");
  }

  virtual void set_inner_cell()
  {
    BCWeak< PhysData >::set_inner_cell();
    cons_flx_pt_solution.resize(inner_cell->get().sf->nb_flx_pts());
    char_flx_pt_solution.resize(inner_cell->get().sf->nb_flx_pts());

    mesh::Field::View sol_pt_solution = solution_field().view(inner_cell->get().space->connectivity()[inner_cell->get().idx]);
    inner_cell->get().reconstruct_from_solution_space_to_flux_points(sol_pt_solution,cons_flx_pt_solution);

    flx_pt_plane_jacobian_normal->cache(cell_entities,cell_idx);

  }
  virtual void unset_inner_cell()
  {
    BCWeak< PhysData >::unset_inner_cell();
    flx_pt_plane_jacobian_normal->get().unlock();
  }

  virtual void compute_solution(const PhysData &inner_cell_data, const RealVectorNDIM &boundary_face_normal, RealVectorNEQS &boundary_face_solution)
  {
    enum {ENTR=0,OMEGA=1,APLUS=2,AMIN=3};


    RealMatrix J((int)NDIM,(int)NDIM);
    RealMatrix Jinv((int)NDIM,(int)NDIM);
    Real detJ;

    RealMatrix nodes;
    inner_cell->get().space->support().geometry_space().allocate_coordinates(nodes);
    inner_cell->get().space->support().geometry_space().put_coordinates(nodes,inner_cell->get().idx);
    inner_cell->get().space->support().element_type().compute_jacobian(inner_cell->get().sf->flx_pts().row(cell_flx_pt),nodes,J);
    Jinv = J.inverse();
    detJ = J.determinant();

//    Real Jxi  = Jinv.row(KSI).norm();
//    Real Jeta = Jinv.row(ETA).norm();

//    std::cout << "Jxi = " << Jxi << "    Jeta = " << Jeta << std::endl;

    outward_normal = flx_pt_plane_jacobian_normal->get().plane_unit_normal[cell_flx_pt] * flx_pt_plane_jacobian_normal->get().sf->flx_pt_sign(cell_flx_pt);

    const RealRowVector& n = outward_normal;
    RealRowVector s((int)NDIM); s << n[YY], -n[XX];

//    std::cout << "n = " << n << std::endl;
//    std::cout << "s = " << s << std::endl;

//    RealVector U0(2);
//    U0[XX] = options().value< std::vector<Real> >("U0")[XX];
//    U0[YY] = options().value< std::vector<Real> >("U0")[YY];
//    Real u0n = U0.dot(n);
//    Real u0s = U0.dot(s);

    RealRowVector mapped_n = n * detJ * Jinv;
    RealRowVector mapped_s = s * detJ * Jinv;

//    std::cout << "mapped_n = " << mapped_n << std::endl;
//    std::cout << "mapped_s = " << mapped_s << std::endl;

//    if ( std::abs(outward_normal[XX]-1.) > 1e-10)
//      throw common::BadValue(FromHere(), "outward_normal incorrect");

    for (Uint f=0; f<inner_cell->get().sf->nb_flx_pts(); ++f)
    {
      cons_to_char(cons_flx_pt_solution[f],outward_normal,char_flx_pt_solution[f]);
    }

    // Extrapolation BC
    char_bdry_solution[ENTR ] = char_flx_pt_solution[cell_flx_pt][ENTR ];
    char_bdry_solution[OMEGA] = char_flx_pt_solution[cell_flx_pt][OMEGA];
    char_bdry_solution[APLUS] = char_flx_pt_solution[cell_flx_pt][APLUS];
    char_bdry_solution[AMIN ] = char_flx_pt_solution[cell_flx_pt][AMIN ];

    sf_deriv_xi.resize(inner_cell->get().sf->nb_flx_pts());
    sf_deriv_eta.resize(inner_cell->get().sf->nb_flx_pts());
    sf_deriv_n.resize(inner_cell->get().sf->nb_flx_pts());
    sf_deriv_s.resize(inner_cell->get().sf->nb_flx_pts());
    inner_cell->get().sf->compute_flux_derivative(KSI,inner_cell->get().sf->flx_pts().row(cell_flx_pt),sf_deriv_xi);
    inner_cell->get().sf->compute_flux_derivative(ETA,inner_cell->get().sf->flx_pts().row(cell_flx_pt),sf_deriv_eta);


    for (Uint j=0; j<inner_cell->get().sf->nb_flx_pts(); ++j)
    {
      sf_deriv_n[j] = mapped_n[KSI] * sf_deriv_xi[j] + mapped_n[ETA] * sf_deriv_eta[j];
      sf_deriv_s[j] = mapped_s[KSI] * sf_deriv_xi[j] + mapped_s[ETA] * sf_deriv_eta[j];
    }

    std::vector<Real> omega(inner_cell->get().sf->nb_flx_pts());
    Real dAmindn = 0.;
    Real dAminds = 0.;
    Real dAplusdn = 0.;
    Real dAplusds = 0.;
    Real dOmegadn = 0.;
    Real dOmegads = 0.;
    Real domegadn = 0.;
    Real domegads = 0.;
    for (Uint j=0; j<inner_cell->get().sf->nb_flx_pts(); ++j)
    {
      dOmegadn   += char_flx_pt_solution[j][OMEGA] * sf_deriv_n[j];
      dOmegads   += char_flx_pt_solution[j][OMEGA] * sf_deriv_s[j];
      dAmindn    += char_flx_pt_solution[j][AMIN ] * sf_deriv_n[j];
      dAminds    += char_flx_pt_solution[j][AMIN ] * sf_deriv_s[j];
      dAplusdn   += char_flx_pt_solution[j][APLUS] * sf_deriv_n[j];
      dAplusds   += char_flx_pt_solution[j][APLUS] * sf_deriv_s[j];

      omega[j] = 0.5*(char_flx_pt_solution[j][APLUS] - char_flx_pt_solution[j][AMIN]);
      domegadn += omega[j] * sf_deriv_n[j];
      domegads += omega[j] * sf_deriv_s[j];
    }



//    dA/dn + dOmega/ds = 0
//    dAplus/dn - dAmin/dn = - 2 dOmega/ds
//    0 < (Aplus-Amin)/(Omega) < 1 --> tangent flow dominates
//    (Aplus-Amin)/(Omega) > 1 --> normal flow dominates

//    std::cout << "gradx = " << sf_deriv_xi.transpose() << std::endl;
//    std::cout << "dAmindxi_internal = " << dAmindxi_internal << std::endl;

    // Try 1
    // dAmin/dt + (u0+c0)dAmin/dn + c0 dAmin/ds - c0 dOmega/ds = 0
//    dAmindn = 1./(m_u0n-m_c0) * ( (m_u0n+m_c0)*dAmindn - 2.*m_c0*dOmegads );

    // Try 2!    dAmin/dt +  u0n dOmega/ds + u0s dAmin/dn = 0
//    dAmindn = dOmegads;

//    dAmindn = -dAplusdn  -->  dAmindn = dOmegads
//    dAmindn/dAplusdn = dAmindn/dOmegads

      // BC divzero
//      dAmindn = dAplusdn + 2*dOmegads;

     // BC dAdt + u0n dAdn = 0 boils to same as divzero when u0s=0
     // dAmindn = 1./m_c0*(u0s*dAminds+u0s*dAplusds+m_c0*dAplusdn+2*m_c0*dOmegads);

//    Aplus*Amin > 0 --> pulse
//    Aplus*Amin < 0 --> vortex

//    Aplus + Amin = 0 --> vortex
//    Aplus - Amin = 0 --> pulse
//    Aplus+Amin =
//    Real ratio = ( std::abs(dAmindn) < 1e-12 ? 0. : dAmindn/dOmegads);

//    static Real i = 0.;
//    static Real avg_ratio = ratio;
//    static Real max_ratio = ratio;
//    static Real min_ratio = ratio;


//    if (ratio != 0 && std::abs(ratio) < 2)
//    {
//      max_ratio = std::max(max_ratio,ratio);
//      min_ratio = std::min(min_ratio,ratio);

//      avg_ratio = i/(i+1.) * avg_ratio  +  1./(i+1.) * ratio;
//      ++i;
////      std::cout << "ratio = " << ratio << "\t  i = " << i << "\t  avg = " << avg_ratio << " \t  max = " << max_ratio << " \t  min = " << min_ratio << std::endl;
//    }

    // vortex: ratio = 1.    alpha = 0
    // pulse:  ratio = 0.5   alpha = 1
//    Real alpha = std::max(0., std::min(1., 2.* (1.-std::abs(ratio)) ));
//    std::cout << "alpha = " << alpha << std::endl;
    Real alpha = options().value<Real>("alpha");
    dAmindn = (1.-alpha)*dOmegads + alpha*dAmindn;

//    dAmindn = dAmindn + 2.*dAmindn/dAplusdn*dOmegads;
//    dAmindn = 0.;//dOmegads;
    // Hedstrom:  dAmin/dt = 0
    // --> (u0n - c0) dAmin/dn + u0s dAmin/ds + c0 dOmega/ds = 0
//    dAmindn = 1./(m_u0n-m_c0)*(-m_u0s*dAminds - m_c0*dOmegads);


    // dAmin/dt + u0n dAmin/dn + u0s dAmin/ds = 0
//    dAmindn = 1./(m_u0n+m_c0)*(m_u0n*dAmindn - m_c0*dOmegads);

    // Try 3
//    dAmindn = 2.*dOmegads + dAplusdn;

//    replace = dAmindxi_internal + 2.*dOmegadeta;
//    replace = -dAmindxi_internal;
//    replace = -dAplusdxi;
//    replace = dAmindxi_internal;

//    replace = 2./(m_u0n-m_c0) * ( m_u0n/2.*dAmindxi_internal  -m_c0/2.*dAplusdxi -m_c0*dOmegadeta );


//    // Also modify equation for omega:
//    dAplusdn = -dAmindn;

//    Real A = 0.5*(char_flx_pt_solution[cell_flx_pt][APLUS] + char_flx_pt_solution[cell_flx_pt][AMIN ]);

//    dAplusdn = -dAmindn;
    char_bdry_solution[AMIN ] = dAmindn;
//    char_bdry_solution[APLUS] = dAplusdn;

    for (Uint j=0; j<inner_cell->get().sf->nb_flx_pts(); ++j)
    {
      if (j!=cell_flx_pt)
      {
        char_bdry_solution[AMIN ] -= char_flx_pt_solution[j][AMIN ] * sf_deriv_n[j];
//        char_bdry_solution[APLUS] -= char_flx_pt_solution[j][APLUS] * sf_deriv_xi[j];
      }
    }
    char_bdry_solution[AMIN ] /= sf_deriv_n[cell_flx_pt];

//    Real A = 0.5*(char_flx_pt_solution[cell_flx_pt][APLUS] + char_flx_pt_solution[cell_flx_pt][AMIN]);
//    char_bdry_solution[APLUS] = 2.*A - char_bdry_solution[AMIN];
//    char_bdry_solution[APLUS] /= sf_deriv_xi[cell_flx_pt];
////    std::cout << char_bdry_solution[AMIN] << std::endl;

//    compute_optimization(char_bdry_solution[OMEGA],char_bdry_solution[APLUS],char_bdry_solution[AMIN]);

//    char_bdry_solution[APLUS] = char_flx_pt_solution[cell_flx_pt][APLUS];
//    compute_optimization_2(char_bdry_solution[OMEGA],char_bdry_solution[AMIN]);



    char_to_cons(char_bdry_solution,outward_normal,boundary_face_solution);

  }


  void cons_to_char(const RealVectorNEQS& conservative, const RealVectorNDIM& characteristic_normal, RealVectorNEQS& characteristic)
  {
    const Real& rho   = conservative[0];
    const Real& rho0u = conservative[1];
    const Real& rho0v = conservative[2];
    const Real& p     = conservative[3];
    const Real& nx    = characteristic_normal[XX];
    const Real& ny    = characteristic_normal[YY];
    Real& S     = characteristic[0];
    Real& Omega = characteristic[1];
    Real& Aplus = characteristic[2];
    Real& Amin  = characteristic[3];

    S     =  rho - p/(m_c0*m_c0);
    Omega =  ny*rho0u - nx*rho0v;
    Aplus =  nx*rho0u + ny*rho0v + p/m_c0;
    Amin  = -nx*rho0u - ny*rho0v + p/m_c0;
  }

  void char_to_cons(const RealVectorNEQS& characteristic, const RealVectorNDIM& characteristic_normal, RealVectorNEQS& conservative)
  {
    const Real& S     = characteristic[0];
    const Real& Omega = characteristic[1];
    const Real& Aplus = characteristic[2];
    const Real& Amin  = characteristic[3];
    const Real& nx    = characteristic_normal[XX];
    const Real& ny    = characteristic_normal[YY];
    Real& rho   = conservative[0];
    Real& rho0u = conservative[1];
    Real& rho0v = conservative[2];
    Real& p     = conservative[3];

    const Real A     = 0.5*(Aplus+Amin);
    const Real omega = 0.5*(Aplus-Amin);

    rho   =  S + A/m_c0;
    rho0u =  ny*Omega + nx*omega;
    rho0v = -nx*Omega + ny*omega;
    p     =  m_c0*A;
  }


  Handle<CacheT<FluxPointPlaneJacobianNormal<NDIM> > > flx_pt_plane_jacobian_normal;
  RealVectorNDIM rhoU;
  RealVectorNDIM outward_normal;
  RealVector sf_deriv_xi;
  RealVector sf_deriv_eta;
  RealVector sf_deriv_n;
  RealVector sf_deriv_s;

  std::vector<RealVectorNEQS> cons_flx_pt_solution;
  std::vector<RealVectorNEQS> char_flx_pt_solution;
  RealVectorNEQS char_bdry_solution;
  Real m_c0;

#if 0
  Real m_u0n;
  Real m_u0s;

  void compute_optimization(Real& Omega, Real& Aplus, Real& Amin)
  {
    enum {ENTR=0,OMEGA=1,APLUS=2,AMIN=3};

    Real dAmindn = 0.;
    Real dAminds = 0.;
    Real dAplusdn = 0.;
    Real dAplusds = 0.;
    Real dOmegadn = 0.;
    Real dOmegads = 0.;
    for (Uint j=0; j<inner_cell->get().sf->nb_flx_pts(); ++j)
    {
      dOmegadn   += char_flx_pt_solution[j][OMEGA] * sf_deriv_xi[j];
      dOmegads   -= char_flx_pt_solution[j][OMEGA] * sf_deriv_eta[j];
      dAmindn    += char_flx_pt_solution[j][AMIN ] * sf_deriv_xi[j];
      dAminds    -= char_flx_pt_solution[j][AMIN ] * sf_deriv_eta[j];
      dAplusdn   += char_flx_pt_solution[j][APLUS] * sf_deriv_xi[j];
      dAplusds   -= char_flx_pt_solution[j][APLUS] * sf_deriv_eta[j];
    }
    Real dOmegadt = m_u0n*dOmegadn + m_u0s*dOmegads + m_c0/2.*(dAplusds + dAminds);

    RealVector3 X;
    X << char_flx_pt_solution[cell_flx_pt][OMEGA],
         char_flx_pt_solution[cell_flx_pt][APLUS],
         char_flx_pt_solution[cell_flx_pt][AMIN];

    RealVector3 F;
    RealMatrix3 J;
    RealVector3 Xnew;

//    std::cout << "\n\n\n" << std::endl;
    Uint iter=0;
    Real error = 1.;
    while (error>1e-7 && iter<15)
    {
      Real dAmindn = 0.;
      Real dAminds = 0.;
      Real dAplusdn = 0.;
      Real dAplusds = 0.;
      Real dOmegadn = 0.;
      Real dOmegads = 0.;
      for (Uint j=0; j<inner_cell->get().sf->nb_flx_pts(); ++j)
      {
        if (j!=cell_flx_pt)
        {
          dOmegadn   += char_flx_pt_solution[j][OMEGA] * sf_deriv_xi[j];
          dOmegads   -= char_flx_pt_solution[j][OMEGA] * sf_deriv_eta[j];
          dAmindn    += char_flx_pt_solution[j][AMIN ] * sf_deriv_xi[j];
          dAminds    -= char_flx_pt_solution[j][AMIN ] * sf_deriv_eta[j];
          dAplusdn   += char_flx_pt_solution[j][APLUS] * sf_deriv_xi[j];
          dAplusds   -= char_flx_pt_solution[j][APLUS] * sf_deriv_eta[j];
        }
      }
      dOmegadn   += X[0] * sf_deriv_xi[cell_flx_pt];
      dOmegads   -= X[0] * sf_deriv_eta[cell_flx_pt];
      dAplusdn   += X[1] * sf_deriv_xi[cell_flx_pt];
      dAplusds   -= X[1] * sf_deriv_eta[cell_flx_pt];
      dAmindn    += X[2] * sf_deriv_xi[cell_flx_pt];
      dAminds    -= X[2] * sf_deriv_eta[cell_flx_pt];


      F << m_u0n*dOmegadn + m_u0s*dOmegads + m_c0/2.*dAplusds + m_c0/2.*dAminds - dOmegadt,
           dAplusdn + dOmegads,
           dAmindn - dOmegads;

      J << m_u0n*sf_deriv_xi[cell_flx_pt] + m_u0s*sf_deriv_eta[cell_flx_pt] ,  m_c0/2.*sf_deriv_eta[cell_flx_pt] , m_c0/2.*sf_deriv_eta[cell_flx_pt],
            sf_deriv_eta[cell_flx_pt]                                       ,  sf_deriv_xi[cell_flx_pt]       , 0.,
           -sf_deriv_eta[cell_flx_pt]                                       ,  0.                             , sf_deriv_xi[cell_flx_pt];

      Xnew.noalias() = X - J.inverse() * F;

      error = ((X-Xnew).array()/X.array()).abs().maxCoeff();


//      std::cout << "iter = " << iter++ << std::endl;
//      std::cout << "error= " << error << std::endl;
//      std::cout << "X    = " << X.transpose() << std::endl;
//      std::cout << "Xnew = " << Xnew.transpose() << std::endl;
//      std::cout << "F    = " << F.transpose() << std::endl;
//      std::cout << "J    = \n" << J << std::endl;



      X = Xnew;
      ++iter;
    }

    Omega = X[0];
    Aplus = X[1];
    Amin  = X[2];
  }

  void compute_optimization_2(Real& Omega, Real& Amin)
  {
    enum {ENTR=0,OMEGA=1,APLUS=2,AMIN=3};

    Real dAmindn = 0.;
    Real dAminds = 0.;
    Real dAplusdn = 0.;
    Real dAplusds = 0.;
    Real dOmegadn = 0.;
    Real dOmegads = 0.;
    for (Uint j=0; j<inner_cell->get().sf->nb_flx_pts(); ++j)
    {
      dOmegadn   += char_flx_pt_solution[j][OMEGA] * sf_deriv_xi[j];
      dOmegads   -= char_flx_pt_solution[j][OMEGA] * sf_deriv_eta[j];
      dAmindn    += char_flx_pt_solution[j][AMIN ] * sf_deriv_xi[j];
      dAminds    -= char_flx_pt_solution[j][AMIN ] * sf_deriv_eta[j];
      dAplusdn   += char_flx_pt_solution[j][APLUS] * sf_deriv_xi[j];
      dAplusds   -= char_flx_pt_solution[j][APLUS] * sf_deriv_eta[j];
    }
    Real dOmegadt = m_u0n*dOmegadn + m_u0s*dOmegads + m_c0/2.*(dAplusds + dAminds);

    RealVector2 X;
    X << char_flx_pt_solution[cell_flx_pt][OMEGA],
         char_flx_pt_solution[cell_flx_pt][AMIN];

    RealVector2 F;
    RealMatrix2 J;
    RealVector2 Xnew;

    Real Aplus = char_flx_pt_solution[cell_flx_pt][APLUS];

//    std::cout << "\n\n\n" << std::endl;
    Uint iter=0;
    Real error = 1.;

    while (error>1e-7 && iter<15)
    {
      Real dAmindn = 0.;
      Real dAminds = 0.;
      Real dAplusdn = 0.;
      Real dAplusds = 0.;
      Real dOmegadn = 0.;
      Real dOmegads = 0.;
      for (Uint j=0; j<inner_cell->get().sf->nb_flx_pts(); ++j)
      {
        if (j!=cell_flx_pt)
        {
          dOmegadn   += char_flx_pt_solution[j][OMEGA] * sf_deriv_xi[j];
          dOmegads   -= char_flx_pt_solution[j][OMEGA] * sf_deriv_eta[j];
          dAmindn    += char_flx_pt_solution[j][AMIN ] * sf_deriv_xi[j];
          dAminds    -= char_flx_pt_solution[j][AMIN ] * sf_deriv_eta[j];
          dAplusdn   += char_flx_pt_solution[j][APLUS] * sf_deriv_xi[j];
          dAplusds   -= char_flx_pt_solution[j][APLUS] * sf_deriv_eta[j];
        }
      }
      dOmegadn   += X[0] * sf_deriv_xi[cell_flx_pt];
      dOmegads   -= X[0] * sf_deriv_eta[cell_flx_pt];
      dAplusdn   += Aplus * sf_deriv_xi[cell_flx_pt];
      dAplusds   -= Aplus * sf_deriv_eta[cell_flx_pt];
      dAmindn    += X[1] * sf_deriv_xi[cell_flx_pt];
      dAminds    -= X[1] * sf_deriv_eta[cell_flx_pt];


      F << m_u0n*dOmegadn + m_u0s*dOmegads + m_c0/2.*dAplusds + m_c0/2.*dAminds - dOmegadt,
           dAmindn - dOmegads;

      J << m_u0n*sf_deriv_xi[cell_flx_pt] + m_u0s*sf_deriv_eta[cell_flx_pt] , m_c0/2.*sf_deriv_eta[cell_flx_pt],
           -sf_deriv_eta[cell_flx_pt]                                       , sf_deriv_xi[cell_flx_pt];

      Xnew.noalias() = X - J.inverse() * F;

      error = ((X-Xnew).array()/X.array()).abs().maxCoeff();


//      std::cout << "iter = " << iter << std::endl;
//      std::cout << "error= " << error << std::endl;
//      std::cout << "X    = " << X.transpose() << std::endl;
//      std::cout << "Xnew = " << Xnew.transpose() << std::endl;
//      std::cout << "F    = " << F.transpose() << std::endl;
//      std::cout << "J    = \n" << J << std::endl;

      X = Xnew;
      ++iter;
    }

    Omega = X[0];
    Amin  = X[1];
  }
#endif
};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_BCSubsonicOutlet2D_hpp
