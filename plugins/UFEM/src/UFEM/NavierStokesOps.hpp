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
    
    const Real he = UT::dimension == 2 ? sqrt(4./3.141592654*u.support().volume()) : ::pow(3./4./3.141592654*u.support().volume(),1./3.);
    const Real ree=coeffs.rho*coeffs.u_ref*he/(2.*coeffs.mu);
    cf3_assert(ree > 0.);
    const Real xi = ree < 3. ? 0.3333333333333333*ree : 1.;
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
      cf3_assert(ree > 0.);
      const Real xi = ree < 3. ? 0.3333333333333333*ree : 1.;
      coeffs.tau_su = h*xi/(2.*umag);
    }
  }
};

/// Placeholder for the compute_tau operation
static solver::actions::Proto::MakeSFOp<ComputeTau>::type const compute_tau = {};

/// Construct element matrix for 2D P1 triangles and 3D P1 Tetrahedra
/// Code from CF2 by Tamás Bányai
struct SUPGSpecialized
{
  typedef void result_type;
  
  template<typename PT, typename UT, typename MatrixT>
  void operator()(const PT& p, const UT& u, SUPGCoeffs& coeffs, MatrixT& A, MatrixT& T)
  {
    typedef typename UT::EtypeT ElementT;
    const RealVector2 u_avg = u.value().colwise().mean();
    const typename ElementT::NodesT& nodes = u.support().nodes();
    const Real volume = u.support().volume();
    const Real fc = 0.5;
    const Real nu = coeffs.one_over_rho*coeffs.mu;
    
    // Face normals
    typename ElementT::NodesT normals;
    normals(0, XX) = nodes(1, YY) - nodes(2, YY);
    normals(0, YY) = nodes(2, XX) - nodes(1, XX);
    normals(1, XX) = nodes(2, YY) - nodes(0, YY);
    normals(1, YY) = nodes(0, XX) - nodes(2, XX);
    normals(2, XX) = nodes(0, YY) - nodes(1, YY);
    normals(2, YY) = nodes(1, XX) - nodes(0, XX);
    
    
    const Real he = sqrt(4./3.141592654*volume);
    const Real ree=coeffs.rho*coeffs.u_ref*he/(2.*coeffs.mu);
    cf3_assert(ree > 0.);
    const Real xi = ree < 3. ? 0.3333333333333333*ree : 1.;
    //coeffs.tau_ps = he*xi/(2.*coeffs.u_ref);
    //coeffs.tau_bulk = he*coeffs.u_ref/xi;

    const Real umag = u_avg.norm();
    //coeffs.tau_su = 0.;
    if(umag > 1e-10)
    {
      const Real h = 2. * volume / (normals * (u_avg / umag)).array().abs().sum();
      Real ree=coeffs.rho*umag*h/(2.*coeffs.mu);
      cf3_assert(ree > 0.);
      const Real xi = ree < 3. ? 0.3333333333333333*ree : 1.;
      //coeffs.tau_su = h*xi/(2.*umag);
    }
    
    for(Uint i=0; i<3; ++i)
    {
      const Uint Pi = i + 3*p.offset;
      const Uint Ui = i + 3*u.offset;
      const Uint Vi = Ui + 3;

      const Real u_ni = u_avg[XX]*normals(i, XX)+u_avg[YY]*normals(i, YY);

      for(Uint j=0; j<3; ++j)
      {
        const Uint Pj = j + 3*p.offset;
        const Uint Uj = j + 3*u.offset;
        const Uint Vj = Uj + 3;
        
        const Real uk=u_avg[XX];
        const Real vk=u_avg[YY];
        const Real uknj=uk*normals(j, XX)+vk*normals(j, YY);
        
        // Convection (Standard + SUPG)
        Real val  = 1./6.*uknj;
        val += coeffs.tau_su/(4.*volume)*uknj*u_ni;
        A(Ui,Uj) += val;
        A(Vi,Vj) += val;

        // Convection (PSPG)
        val = coeffs.tau_ps/(4.*volume)*uknj;
        A(Pi,Uj) += val*normals(i, XX);
        A(Pi,Vj) += val*normals(i, YY);

        // Convection Skewsymm (Standard + SUPG)
        val  = fc/6.;
        val += fc*coeffs.tau_su/(4.*volume)*u_ni;
        A(Ui,Uj) += val*uk*normals(j, XX);
        A(Ui,Vj) += val*uk*normals(j, YY);
        A(Vi,Uj) += val*vk*normals(j, XX);
        A(Vi,Vj) += val*vk*normals(j, YY);

        // Convection Skewsymm (PSPG)
        val = fc*coeffs.tau_ps/(4.*volume);
        A(Pi,Uj) += val*normals(i, XX)*uk*normals(j, XX);
        A(Pi,Vj) += val*normals(i, XX)*uk*normals(j, YY);
        A(Pi,Uj) += val*normals(i, YY)*vk*normals(j, XX);
        A(Pi,Vj) += val*normals(i, YY)*vk*normals(j, YY);

        //difusion (Standard)
        val = nu/(4.*volume);
        A(Ui,Uj)+=val*(4./3.*normals(i, XX)*normals(j, XX)+      normals(i, YY)*normals(j, YY));
//         A(Ui,Vj)+=val*       normals(i, YY)*normals(j, XX);
//         A(Vi,Uj)+=val*       normals(i, XX)*normals(j, YY);
        A(Ui,Vj)+=val*       normals(i, XX)*normals(j, YY)/3.;
        A(Vi,Uj)+=val*       normals(i, YY)*normals(j, XX)/3.;        
        A(Vi,Vj)+=val*(      normals(i, XX)*normals(j, XX)+4./3.*normals(i, YY)*normals(j, YY));
        
        // Pressure (Standard + SUPG)
        val  = 1./(6.*coeffs.rho);
        val += coeffs.tau_su/(4.*coeffs.rho*volume)*u_ni;
        A(Ui,Pj) += normals(j, XX)*val;
        A(Vi,Pj) += normals(j, YY)*val;

        // Pressure (PSPG)
        const Real laplacian = 1./(4.*coeffs.rho*volume)*(normals(i, XX)*normals(j, XX)+normals(i, YY)*normals(j, YY));
        A(Pi,Pj) += coeffs.tau_ps*laplacian;

        // Continuity (Standard)
        val = 1./6.;
        A(Pi,Uj) += val*normals(j, XX);
        A(Pi,Vj) += val*normals(j, YY);

        // Bulk viscosity (Standard)
        val = coeffs.tau_bulk/(4.*volume);
        A(Ui,Uj) += val*normals(i, XX)*normals(j, XX);
        A(Ui,Vj) += val*normals(i, XX)*normals(j, YY);
        A(Vi,Uj) += val*normals(i, YY)*normals(j, XX);
        A(Vi,Vj) += val*normals(i, YY)*normals(j, YY);

        // Time (Standard + SUPG)
        val  = volume/12.*(1.+((i)==(j)?1.:0.));
        val += coeffs.tau_su/6.*u_ni;
        T(Ui,Uj) += val;
        T(Vi,Vj) += val;

        // Time (PSPG)
        val = coeffs.tau_ps/6.;
        T(Pi,Uj) += val*normals(i, XX);
        T(Pi,Vj) += val*normals(i, YY);

        // Increasing diagonal dominance - crank nicholson related
//         A(Pi,Pj) *= 2.;
//         A(Pi,Uj) *= 2.;
//         A(Pi,Vj) *= 2.;
      }
    }
  }
};

/// Placeholder for the specialized ops
static solver::actions::Proto::MakeSFOp<SUPGSpecialized>::type const supg_specialized = {};

} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokesOps_hpp
