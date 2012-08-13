// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_SubsonicInFlowWeakBc_hpp
#define cf3_RDM_SubsonicInFlowWeakBc_hpp

#include <iostream> // to remove

#include "math/VectorialFunction.hpp"

#include "RDM/BoundaryTerm.hpp"
#include "RDM/BcBase.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace mesh { class Mesh; class Field; }

namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_API SubsonicInFlowWeakBc : public RDM::BoundaryTerm {
public: // typedefs

  /// the actual BC implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Term;

  /// pointers



public: // functions
  /// Contructor
  /// @param name of the component
  SubsonicInFlowWeakBc ( const std::string& name );

  /// Virtual destructor
  virtual ~SubsonicInFlowWeakBc() {}

  /// Get the class name
  static std::string type_name () { return "SubsonicInFlowWeakBc"; }

  /// execute the action
  virtual void execute ();

  virtual bool is_weak() const { return true; }

private: // helper functions

  void config_density_function();
  void config_velocity_function();

public: // data

  /// function parser to set the value of density
  math::VectorialFunction  density_function;
  /// function parser to set the value of velocity
  math::VectorialFunction  velocity_function;

}; // !SubsonicInFlowWeakBc

//------------------------------------------------------------------------------------------

template < typename SF, typename QD, typename PHYS >
class RDM_API SubsonicInFlowWeakBc::Term : public BcBase<SF,QD,PHYS> {

public: // typedefs

  /// base class type
  typedef BcBase<SF,QD,PHYS> B;
  /// pointers



public: // functions

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  /// Contructor
  /// @param name of the component
 Term ( const std::string& name ) :
   BcBase<SF,QD,PHYS>(name),
   m_quadrature( QD::instance() )

 {
   regist_typeinfo(this);

   // Gradient of the shape functions in reference space
   typename SF::SF::GradientT GradSF;
   // Values of shape functions in reference space
   typename SF::SF::ValueT ValueSF;

   // initialize the interpolation matrix

   for(Uint q = 0; q < QD::nb_points; ++q)
     for(Uint n = 0; n < SF::nb_nodes; ++n)
     {
        SF::SF::compute_gradient( m_quadrature.coords.col(q), GradSF  );
        SF::SF::compute_value   ( m_quadrature.coords.col(q), ValueSF );

        Ni(q,n)     = ValueSF[n];
        dNdKSI(q,n) = GradSF[n];
     }

     vars.resize(DIM_3D);
     rho_in.resize(1);
     vel_in.resize(DIM_3D);
 }

 /// Get the class name
 static std::string type_name () { return "SubsonicInFlowWeakBc.Term<" + SF::type_name() + "," + PHYS::type_name() + ">"; }

protected: // data

 /// variables to pass to vectorial function
 std::vector<Real> vars;

 /// Values used to compute ghost state
 /// @remark rho_in is a RealVector because
 /// that's required by VectorialFunction density_function
 /// Is there a ScalarFunction as well??? Didn't find it.
 /// Same problem for p_out in SubsonicOutFlowWeakBc. Martin
 RealVector rho_in;
 RealVector vel_in;

 /// helper object to compute the quadrature information
 const QD& m_quadrature;

 typedef Eigen::Matrix<Real, QD::nb_points, 1u>               WeightVT;

 typedef typename SF::NodesT                             NodeMT;

 typedef Eigen::Matrix<Real, QD::nb_points, SF::nb_nodes >     SFMatrixT;
 typedef Eigen::Matrix<Real, QD::nb_points, PHYS::MODEL::_ndim   >     QCoordMT;
 typedef Eigen::Matrix<Real, SF::nb_nodes,  PHYS::MODEL::_neqs   >     SolutionMT;
 typedef Eigen::Matrix<Real, QD::nb_points, PHYS::MODEL::_neqs   >     QSolutionMT;
 typedef Eigen::Matrix<Real, PHYS::MODEL::_neqs,    PHYS::MODEL::_ndim   >     QSolutionVT;
 typedef Eigen::Matrix<Real, PHYS::MODEL::_neqs,    1u           >     SolutionVT;
 typedef Eigen::Matrix<Real, PHYS::MODEL::_neqs,    PHYS::MODEL::_ndim   >     FluxMT;
 typedef Eigen::Matrix<Real, PHYS::MODEL::_ndim,    1u           >     DimVT;

 /// derivative matrix - values of shapefunction derivative in Ksi at each quadrature point
 SFMatrixT  dNdKSI;
 /// interporlation matrix - values of shapefunction at each quadrature point
 SFMatrixT  Ni;
 /// node values
 NodeMT     X_n;
 /// coordinates of quadrature points in physical space
 QCoordMT   X_q;
 /// derivatives of coordinate transformation physical->reference space
 QCoordMT  dX_q;
 /// Values of the solution located in the dof of the element
 SolutionMT U_n;
 /// solution at quadrature points in physical space
 QSolutionMT U_q;
 /// derivatives of solution to X at each quadrature point, one matrix per dimension
 QSolutionMT dUdX[PHYS::MODEL::_ndim];
 /// derivatives of solution with respect to x,y,z at ONE quadrature point
 QSolutionVT dUdXq;
 /// Integration factor (jacobian multiplied by quadrature weight)
 WeightVT wj;
 /// contribution to nodal residuals
 SolutionMT Phi_n;

 /// flux computed with current solution
 FluxMT Fu_h;
 /// flux computed with boundary value
 FluxMT Fu_g;
 /// ghost solution vector
 SolutionVT u_g;

 /// diagonal matrix with eigen values
 typename B::PhysicsVT Dv;
 /// diagonal matrix with positive eigen values
 typename B::PhysicsVT DvPlus;

 /// temporary normal on 1 quadrature point
 DimVT dN;

public: // functions

 /// virtual interface to execute the action
 virtual void execute () { executeT(); }

 /// execute the action
 void executeT ()
 {
   // std::cout << "Face [" << B::idx() << "]" << std::endl;

   // get face connectivity

   const mesh::Connectivity::ConstRow nodes_idx = (*B::connectivity)[B::idx()];

   // copy the coordinates from the large array to a small

   mesh::fill(X_n, *B::coordinates, nodes_idx );

   // copy the solution from the large array to a small

   for(Uint n = 0; n < SF::nb_nodes; ++n)
     for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
       U_n(n,v) = (*B::solution)[ nodes_idx[n] ][v];

   // coordinates of quadrature points in physical space

   X_q  = Ni * X_n;

   // derivatives of coordinate transf.

   dX_q = dNdKSI * X_n;

   // solution at all quadrature points in physical space

   U_q = Ni * U_n;

   // zero element residuals

   Phi_n.setZero();

   // loop on quadrature points

   for(Uint q=0; q < QD::nb_points; ++q)
   {

    for(Uint dim = 0; dim < PHYS::MODEL::_ndim; ++dim)
    {
      dUdXq.col(dim) = dUdX[dim].row(q).transpose();
    }

    const Real jacob = std::sqrt( dX_q(q,XX)*dX_q(q,XX)+dX_q(q,YY)*dX_q(q,YY) );

    wj[q] = jacob * m_quadrature.weights[q];

    // compute the normal at quadrature point

    dN[XX] = -dX_q(q,YY)/jacob;
    dN[YY] =  dX_q(q,XX)/jacob;

    // compute the flux F(u_h) and its correction F(u_g)

    PHYS::compute_properties(X_q.row(q),
                             U_q.row(q),
                             dUdXq,
                             B::phys_props);

    PHYS::flux(B::phys_props, Fu_h);

    // compute the flux according to the ghost solution u_g

    vars[XX] = X_q(q,XX);
    vars[YY] = X_q(q,YY);
    vars[ZZ] = 0.0;

    this->parent()->template handle<SubsonicInFlowWeakBc>()->density_function.evaluate(vars,rho_in);
    this->parent()->template handle<SubsonicInFlowWeakBc>()->velocity_function.evaluate(vars,vel_in);

//    std::cout << "The value rho_in = " << rho_in[0] << std::endl;
//    std::cout << "The value vel_in = " << vel_in[XX] << "," << vel_in[YY] << std::endl;
//    std::cin.get();

    u_g[0] = rho_in[0];
    u_g[1] = rho_in[0]*vel_in[XX];
    u_g[2] = rho_in[0]*vel_in[YY];
    u_g[3] = B::phys_props.P/(B::phys_props.gamma-1.)
                   + 0.5*rho_in[0]*(vel_in[XX]*vel_in[XX]+vel_in[YY]*vel_in[YY]);

    PHYS::compute_properties(X_q.row(q),
                             u_g,
                             dUdXq,
                             B::phys_props);

    PHYS::flux(B::phys_props, Fu_g);

    for(Uint n=0; n < SF::nb_nodes; ++n)
      for(Uint v=0; v < PHYS::MODEL::_neqs; ++v)
      {
        Phi_n.row(n)[v] -= ( ( Fu_g(v,XX) - Fu_h(v,XX) ) * dN[XX] +
                             ( Fu_g(v,YY) - Fu_h(v,YY) ) * dN[YY] )
                           *   Ni(q,n) * wj[q];
      }


    // compute the wave_speed for scaling the update

    PHYS::flux_jacobian_eigen_values(B::phys_props,
                                dN,
                                Dv );

    DvPlus = Dv.unaryExpr(std::ptr_fun(plus));

    for(Uint n = 0; n < SF::nb_nodes; ++n)
      (*B::wave_speed)[nodes_idx[n]][0] += DvPlus.array().maxCoeff() * wj[q];


   } //Loop over quadrature points

   // update the residual

   for (Uint n=0; n<SF::nb_nodes; ++n)
     for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
       (*B::residual)[nodes_idx[n]][v] += Phi_n(n,v);

 }

}; // !SubsonicInFlowWeakBc::Term

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_RDM_SubsonicInFlowWeakBc_hpp
