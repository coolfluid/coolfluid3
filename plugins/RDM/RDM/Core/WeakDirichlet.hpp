// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_WeakDirichlet_hpp
#define CF_RDM_WeakDirichlet_hpp

#include <iostream> // to remove

#include "Math/VectorialFunction.hpp"

#include "RDM/Core/BoundaryTerm.hpp"
#include "RDM/Core/BcBase.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Mesh { class CMesh; class CField; }

namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_CORE_API WeakDirichlet : public RDM::BoundaryTerm {
public: // typedefs

  /// the actual BC implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Term;

  /// pointers
  typedef boost::shared_ptr<WeakDirichlet> Ptr;
  typedef boost::shared_ptr<WeakDirichlet const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  WeakDirichlet ( const std::string& name );

  /// Virtual destructor
  virtual ~WeakDirichlet() {};

  /// Get the class name
  static std::string type_name () { return "WeakDirichlet"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_mesh();
  void config_function();

public: // data

  /// access to the solution field on the mesh
  boost::weak_ptr<Mesh::CField> solution;
  /// function parser for the math formula of the dirichlet condition
  Math::VectorialFunction  function;

}; // !WeakDirichlet

//------------------------------------------------------------------------------------------

template < typename SF, typename QD, typename PHYS >
class RDM_CORE_API WeakDirichlet::Term : public BcBase<SF,QD,PHYS> {

public: // typedefs

  /// base class type
  typedef BcBase<SF,QD,PHYS> B;
  /// pointers
  typedef boost::shared_ptr< Term > Ptr;
  typedef boost::shared_ptr< Term const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
 Term ( const std::string& name ) :
   BcBase<SF,QD,PHYS>(name),
   m_quadrature( QD::instance() )

 {
   regist_typeinfo(this);

   // std::cout << "SF        ::type_name()    [" << SF::type_name()   << "]" << std::endl << std::flush;
   // std::cout << "SF        ::nb_nodes       [" << SF::nb_nodes      << "]" << std::endl << std::flush;
   // std::cout << "QD        ::nb_points      [" << QD::nb_points     << "]" << std::endl << std::flush;
   // std::cout << "PHYS      ::ndim           [" << PHYS::ndim        << "]" << std::endl << std::flush;
   // std::cout << "PHYS      ::neqs           [" << PHYS::neqs        << "]" << std::endl << std::flush;
   // std::cout << "PHYS      ::type_name()    [" << PHYS::type_name() << "]" << std::endl << std::flush;


   // Gradient of the shape functions in reference space
   typename SF::MappedGradientT GradSF;
   // Values of shape functions in reference space
   typename SF::ShapeFunctionsT ValueSF;

   // initialize the interpolation matrix

   for(Uint q = 0; q < QD::nb_points; ++q)
     for(Uint n = 0; n < SF::nb_nodes; ++n)
     {
       // std::cout << FromHere().str() << " " << q << " " << n << std::endl << std::flush;

//         SF::mapped_gradient( m_quadrature.coords.col(q), GradSF  );
        SF::shape_function ( m_quadrature.coords.col(q), ValueSF );

        Ni(q,n) = ValueSF[n];

//        for(Uint d = 0; d < PHYS::ndim; ++d)
//          dNdKSI[d](q,n) = GradSF(d,n);
     }

   // std::cout << FromHere().str() << std::endl << std::flush;

   vars.resize(DIM_3D);

   /// @note hardocded for scalar
   return_val.resize(1);
 }

 /// Get the class name
 static std::string type_name () { return "WeakDirichlet.BC<" + SF::type_name() + ">"; }

protected: // data

 /// valriables to pass to vectorial function
 std::vector<Real> vars;
 /// return value from the vectorial function
 RealVector return_val;

 /// helper object to compute the quadrature information
 const QD& m_quadrature;

 typedef Eigen::Matrix<Real, QD::nb_points, 1u>               WeightVT;

 typedef typename SF::NodeMatrixT                             NodeMT;

 typedef Eigen::Matrix<Real, QD::nb_points, SF::nb_nodes>     SFMatrixT;
 typedef Eigen::Matrix<Real, QD::nb_points, PHYS::ndim>       QCoordMT;
 typedef Eigen::Matrix<Real, SF::nb_nodes,  PHYS::neqs>       SolutionMT;
 typedef Eigen::Matrix<Real, QD::nb_points, PHYS::neqs>       QSolutionMT;

 /// derivative matrix - values of shapefunction derivative in Ksi at each quadrature point
 SFMatrixT  dNdKSI[PHYS::ndim];
 /// interporlation matrix - values of shapefunction at each quadrature point
 SFMatrixT  Ni;
 /// node values
 NodeMT     X_n;
 /// coordinates of quadrature points in physical space
 QCoordMT   X_q;
 /// Values of the solution located in the dof of the element
 SolutionMT U_n;
 /// solution at quadrature points in physical space
 QSolutionMT U_q;
 /// Integration factor (jacobian multiplied by quadrature weight)
 WeightVT wj;
 /// contribution to nodal residuals
 SolutionMT Phi_n;

public: // functions

 /// virtual interface to execute the action
 virtual void execute () { executeT(); }

 /// execute the action
 void executeT ()
 {
   // std::cout << "Face [" << B::idx() << "]" << std::endl;

   // get face connectivity

   const Mesh::CTable<Uint>::ConstRow nodes_idx = this->connectivity_table->array()[B::idx()];

//   const Uint nbnodes = nodes_idx.shape()[1];
//   for( Uint n = 0; n < nbnodes; ++n )
//   {
//     // std::cout << nodes_idx[n] << " ";
//   }
//   // std::cout << std::endl;

   // copy the coordinates from the large array to a small

   Mesh::fill(X_n, *B::coordinates, nodes_idx );

   // copy the solution from the large array to a small

   for(Uint n = 0; n < SF::nb_nodes; ++n)
     for (Uint v=0; v < PHYS::neqs; ++v)
       U_n(n,v) = (*B::solution)[ nodes_idx[n] ][v];

   // coordinates of quadrature points in physical space

   X_q  = Ni * X_n;

   // solution at all quadrature points in physical space

   U_q = Ni * U_n;

   // zero element residuals

   Phi_n.setZero();

   // ------------------------------------------------------
   /// @note lagrange P1 line specifc

   const Real x1x0 =  X_n(1,XX) - X_n(0,XX) ;
   const Real y1y0 =  X_n(1,YY) - X_n(0,YY) ;

   const Real lenght = sqrt(x1x0*x1x0 + y1y0*y1y0);

   const Real jacob = lenght * 0.5 ;

   // std::cout << "jacob [" << jacob << "]" << std::endl;

   // compute transformed integration weights (sum is element area)

   for(Uint q = 0; q < QD::nb_points; ++q)
     wj[q] = jacob * m_quadrature.weights[q];

   const Real nx = -y1y0 / lenght;
   const Real ny =  x1x0 / lenght;

   // std::cout << "n [" << nx << "," << ny << "]" << std::endl;


   for(Uint q=0; q < QD::nb_points; ++q)
   {
     // compute the flux F(u_h) and its correction F(u_g)

     /// @note fixed for scalar inflow with adv. speed (
     const Real Fu_h_x = 1.0 * U_q(q,0);
     const Real Fu_h_y = 1.0 * U_q(q,0);

     // std::cout << "Fu_h [" << Fu_h_x << "," << Fu_h_y << "]" << std::endl;


     vars[XX] = X_q(q,XX);
     vars[YY] = X_q(q,YY);
     vars[ZZ] = 0.0;

     this->parent()->as_type<WeakDirichlet>().function.evaluate(vars,return_val);

     const Real Fu_g_x = 1.0 * return_val[0];
     const Real Fu_g_y = 1.0 * return_val[0];

     // std::cout << "Fu_g [" << Fu_g_x << "," << Fu_g_y << "]" << std::endl;

     for(Uint n=0; n < SF::nb_nodes; ++n)
     {
       Phi_n.row(n)[0] -= ( ( Fu_g_x - Fu_h_x ) * nx + ( Fu_g_y - Fu_h_y ) * ny ) * Ni(q,n) * wj[q];
     }

     // compute the wave_speed for scaling the update

     for(Uint n = 0; n < SF::nb_nodes; ++n)
       (*B::wave_speed)[nodes_idx[n]][0] += 1.0 * wj[q];

   }

   // std::cout << "Phi_n [" << Phi_n << "]" << std::endl;


   // ------------------------------------------------------

   // update the residual

   for (Uint n=0; n<SF::nb_nodes; ++n)
     for (Uint v=0; v < PHYS::neqs; ++v)
       (*B::residual)[nodes_idx[n]][v] += Phi_n(n,v);

//   if (B::idx() > 3) exit(0);
 }

}; // !WeakDirichlet::Term

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_WeakDirichlet_hpp
