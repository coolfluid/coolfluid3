// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_ConvectionNonUniformMeanflow2D_hpp
#define cf3_sdm_lineuler_ConvectionNonUniformMeanflow2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "sdm/ConvectiveTerm.hpp"
#include "sdm/lineuler/LibLinEuler.hpp"
#include "Physics/LinEuler/Cons2D.hpp"



////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

////////////////////////////////////////////////////////////////////////////////

struct NonUniformPhysData2D : PhysDataBase<4,2u>
{
  RealVectorNDIM U0;
  Real rho0;
  Real p0;
};

////////////////////////////////////////////////////////////////////////////////

class sdm_lineuler_API ConvectionNonUniformMeanflow2D : public ConvectiveTerm< NonUniformPhysData2D >
{
private:
  typedef physics::LinEuler::Cons2D PHYS;

public:
  static std::string type_name() { return "ConvectionNonUniformMeanflow2D"; }
  ConvectionNonUniformMeanflow2D(const std::string& name) : ConvectiveTerm< PhysData >(name)
  {
    p.gamma = 1.4;
    options().add("gamma",p.gamma)
        .description("Specific heat reatio")
        .attach_trigger( boost::bind( &ConvectionNonUniformMeanflow2D::config_constants, this) );

    options().add("mean_flow",m_meanflow)
        .description("Field containing mean flow (rho0, U0[x], U0[y], p0)")
        .link_to(&m_meanflow);

    p.rho0 = 1.;
    p.u0.setZero();
    p.P0 = 1.;
    p.c = 1.4;
    p.inv_c = 1./p.c;
  }

  virtual void compute_flx_pt_phys_data(const SFDElement& elem, const Uint flx_pt, PhysData& phys_data )
  {
    ConvectiveTerm<PhysData>::compute_flx_pt_phys_data(elem,flx_pt,phys_data);
    mesh::Field::View sol_pt_meanflow = m_meanflow->view(elem.space->connectivity()[elem.idx]);
    elem.reconstruct_from_solution_space_to_flux_points[flx_pt](sol_pt_meanflow,meanflow);
    phys_data.rho0   = meanflow[0];
    phys_data.U0[XX] = meanflow[1];
    phys_data.U0[YY] = meanflow[2];
    phys_data.p0     = meanflow[3];
  }


  virtual void compute_sol_pt_phys_data(const SFDElement& elem, const Uint sol_pt, PhysData& phys_data )
  {
    ConvectiveTerm<PhysData>::compute_sol_pt_phys_data(elem,sol_pt,phys_data);
    mesh::Field::View sol_pt_meanflow = m_meanflow->view(elem.space->connectivity()[elem.idx]);

    phys_data.rho0   = sol_pt_meanflow[sol_pt][0];
    phys_data.U0[XX] = sol_pt_meanflow[sol_pt][1];
    phys_data.U0[YY] = sol_pt_meanflow[sol_pt][2];
    phys_data.p0     = sol_pt_meanflow[sol_pt][3];
  }

  void config_constants()
  {
    p.gamma = options().option("gamma").value<Real>();
//    p.rho0  = options().option("rho0").value<Real>();
//    p.P0  = options().option("p0").value<Real>();

//    p.inv_rho0 = 1./p.rho0;

//    p.c=sqrt(p.gamma*p.P0*p.inv_rho0);
//    p.inv_c = 1./p.c;

//    std::vector<Real> U0 = options().option("U0").value<std::vector<Real> >();
//    for (Uint d=0; d<U0.size(); ++d)
//      p.u0[d] = U0[d];
  }

  virtual ~ConvectionNonUniformMeanflow2D() {}


  void set_meanflow_properties(const PhysData& phys_data)
  {
    p.rho0 = phys_data.rho0;
    p.u0 = phys_data.U0;
    p.P0 = phys_data.p0;
    p.inv_rho0 = 1./p.rho0;
    p.c=sqrt(p.gamma*p.P0*p.inv_rho0);
    p.inv_c = 1./p.c;
  }

  virtual void compute_analytical_flux(PhysData& data, const RealVectorNDIM& unit_normal,
                                       RealVectorNEQS& flux, Real& wave_speed)
  {
    set_meanflow_properties(data);
    PHYS::compute_properties(data.coord, data.solution , dummy_grads, p);
    PHYS::flux(p, unit_normal, flux);
    PHYS::flux_jacobian_eigen_values(p, unit_normal, eigenvalues);
    wave_speed = eigenvalues.cwiseAbs().maxCoeff();
  }

  virtual void compute_numerical_flux(PhysData& left, PhysData& right, const RealVectorNDIM& unit_normal,
                                      RealVectorNEQS& flux, Real& wave_speed)
  {
    set_meanflow_properties(left);

    // Compute left and right fluxes
    PHYS::compute_properties(left.coord, left.solution, dummy_grads, p);
    PHYS::flux(p , unit_normal, flux_left);

    PHYS::compute_properties(left.coord, right.solution, dummy_grads, p);
    PHYS::flux(p , unit_normal, flux_right);

    // Compute the averaged properties
    sol_avg.noalias() = 0.5*(left.solution+right.solution);
    PHYS::compute_properties(left.coord, sol_avg, dummy_grads, p);

    // Compute absolute jacobian using averaged properties
    PHYS::flux_jacobian_eigen_structure(p,unit_normal,right_eigenvectors,left_eigenvectors,eigenvalues);
    abs_jacobian.noalias() = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;

    // flux = central flux - upwind flux
    flux.noalias() = 0.5*(flux_left + flux_right);
    flux.noalias() -= 0.5*abs_jacobian*(right.solution-left.solution);
    wave_speed = eigenvalues.cwiseAbs().maxCoeff();
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:

  Handle<mesh::Field> m_meanflow;

  PHYS::MODEL::Properties p;
  PHYS::MODEL::Properties p_left;
  PHYS::MODEL::Properties p_right;

  PHYS::MODEL::SolM dummy_grads;
  PHYS::MODEL::GeoV dummy_coords;

  PHYS::MODEL::SolV sol_avg;

  PHYS::MODEL::SolV flux_left;
  PHYS::MODEL::SolV flux_right;

  PHYS::MODEL::SolV eigenvalues;
  PHYS::MODEL::JacM right_eigenvectors;
  PHYS::MODEL::JacM left_eigenvectors;
  PHYS::MODEL::JacM  abs_jacobian;

  RealVectorNEQS meanflow;
};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_ConvectionNonUniformMeanflow2D_hpp
