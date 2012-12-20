// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_SUPG_hpp
#define cf3_UFEM_SUPG_hpp

#include "math/MatrixTypes.hpp"

#include "solver/actions/Proto/ElementOperations.hpp"
#include <solver/actions/Proto/ElementData.hpp>

#include "NavierStokesPhysics.hpp"

namespace cf3 {

namespace UFEM {

/// Helper struct to get the face normals of an element
template<typename ElementT>
struct ElementNormals
{
  typedef Eigen::Matrix<Real, ElementT::nb_faces, ElementT::dimension> NormalsT;

  void operator()(const typename ElementT::NodesT& nodes, NormalsT& normals)
  {
    const mesh::ElementType::FaceConnectivity& face_conn = ElementT::faces();
    const mesh::ElementType& face_etype = ElementT::face_type(0);
    const Uint nb_face_nodes = face_etype.nb_nodes();
    RealMatrix face_nodes(nb_face_nodes, ElementT::dimension);
    RealVector normal(ElementT::dimension);
    for(Uint i = 0; i != ElementT::nb_faces; ++i)
    {
      for(Uint j = 0; j != nb_face_nodes; ++j)
        face_nodes.row(j) = nodes.row(face_conn.nodes[nb_face_nodes*i+j]);
      face_etype.compute_normal(face_nodes, normal);
      normals.row(i) = face_etype.area(face_nodes) * normal;
    }
  }
};

/// Calculation of the stabilization coefficients for the SUPG method
struct ComputeTau
{
  typedef void result_type;

  /// Compute the coefficients for the full Navier-Stokes equations
  template<typename UT, typename NUT>
  void operator()(const UT& u, const NUT& nu_eff, const Real& u_ref, Real& tau_ps, Real& tau_su, Real& tau_bulk) const
  {
    typedef typename UT::EtypeT ElementT;

    // Average viscosity
    Real element_nu = fabs(nu_eff.value().mean());

    const Real he = UT::dimension == 2 ? sqrt(4./3.141592654*u.support().volume()) : ::pow(3./4./3.141592654*u.support().volume(),1./3.);
    const Real ree=u_ref*he/(2.*element_nu);
    cf3_assert(ree > 0.);
    const Real xi = ree < 3. ? 0.3333333333333333*ree : 1.;
    tau_ps = he*xi/(2.*u_ref);
    tau_bulk = he*u_ref/xi;
    tau_su = compute_tau_su(u, element_nu);
  }

  /// Only compute the SUPG coefficient
  template<typename UT, typename NUT>
  void operator()(const UT& u, const NUT& nu_eff, Real& tau_su) const
  {
    tau_su = compute_tau_su(u, fabs(nu_eff.value().mean()));
  }

  template<typename UT>
  Real compute_tau_su(const UT& u, const Real& element_nu) const
  {
    typedef typename UT::EtypeT ElementT;

    // Average cell velocity
    const typename ElementT::CoordsT u_avg = u.value().colwise().mean();
    const Real umag = u_avg.norm();

    if(umag > 1e-10)
    {
      typename ElementNormals<ElementT>::NormalsT normals;
      ElementNormals<ElementT>()(u.support().nodes(), normals);
      const Real h = 2. * u.support().volume() / (normals * (u_avg / umag)).array().abs().sum();
      Real ree=umag*h/(2.*element_nu);
      cf3_assert(ree > 0.);
      const Real xi = ree < 3. ? 0.3333333333333333*ree : 1.;
      return h*xi/(2.*umag);
    }

    return 0.;
  }
};

/// Placeholder for the compute_tau operation. Use as compute_tau(velocity_field, nu_eff_field, u_ref, tau_ps_field, tau_su_field, tau_bulk_field)
static solver::actions::Proto::MakeSFOp<ComputeTau>::type const compute_tau = {};

} // UFEM
} // cf3


#endif // cf3_UFEM_SUPG_hpp
