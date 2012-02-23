// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_NavierStokesOps_hpp
#define cf3_UFEM_NavierStokesOps_hpp

#include "math/MatrixTypes.hpp"

#include "solver/actions/Proto/ElementOperations.hpp"
#include "solver/actions/Proto/Terminals.hpp"

namespace cf3 {
namespace UFEM {

/// Stores the coefficients for the SUPG model and shares them inside a proto expression through the state
struct SUPGCoeffs
{
  /// Reference velocity magnitude
  Real u_ref;

  /// Kinematic viscosity
  Real mu;

  /// Density
  Real rho;

  /// Inverse density
  Real one_over_rho;

  /// Model coefficients
  Real tau_ps, tau_su, tau_bulk;
};

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

struct ComputeTau
{
  typedef void result_type;

  template<typename UT>
  void operator()(const UT& u, SUPGCoeffs& coeffs) const
  {
    typedef typename UT::EtypeT ElementT;
    
    const Real he = UT::dimension == 2 ? sqrt(4./3.141592654*u.support().volume()) : pow(3./4./3.141592654*u.support().volume(),1./3.);
    const Real ree=coeffs.rho*coeffs.u_ref*he/(2.*coeffs.mu);
    const Real xi=std::max(0.,std::min(ree/3.,1.));
    coeffs.tau_ps = he*xi/(2.*coeffs.u_ref);
    coeffs.tau_bulk = he*coeffs.u_ref/xi;

    // Average cell velocity
    const typename ElementT::CoordsT u_avg = u.value().colwise().mean();
    const Real umag = u_avg.norm();
    coeffs.tau_su = 0.;
    if(umag > 1e-10)
    {
      typename ElementNormals<ElementT>::NormalsT normals;
      ElementNormals<ElementT>()(u.support().nodes(), normals);
      const Real h = 2. * u.support().volume() / (normals * (u_avg / umag)).array().abs().sum();
      Real ree=coeffs.rho*umag*h/(2.*coeffs.mu);
      Real xi=std::max(0.,std::min(ree/3.,1.));
      coeffs.tau_su = h*xi/(2.*umag);
    }
  }
};

/// Placeholder for the compute_tau operation
static solver::actions::Proto::MakeSFOp<ComputeTau>::type const compute_tau = {};

} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokesOps_hpp
