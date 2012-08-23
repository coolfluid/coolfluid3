// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_WeakDirichlet_hpp
#define cf3_RDM_WeakDirichlet_hpp

#include "math/VectorialFunction.hpp"

#include "mesh/Elements.hpp"
#include "mesh/FaceCellConnectivity.hpp"

#include "RDM/BoundaryTerm.hpp"
#include "RDM/BcBase.hpp"

namespace cf3 {

namespace mesh { class Mesh; class Field; }

namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_API WeakDirichlet : public RDM::BoundaryTerm {
public: // typedefs

  /// the actual BC implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Term;

  /// pointers



public: // functions
  /// Contructor
  /// @param name of the component
  WeakDirichlet ( const std::string& name );

  /// Virtual destructor
  virtual ~WeakDirichlet() {}

  /// Get the class name
  static std::string type_name () { return "WeakDirichlet"; }

  /// execute the action
  virtual void execute ();

  virtual bool is_weak() const { return true; }

private: // helper functions

  void config_function();

public: // data

  /// access to the solution field on the mesh
  Handle<mesh::Field> solution;
  /// function parser for the math formula of the dirichlet condition
  math::VectorialFunction  function;

}; // !WeakDirichlet

//------------------------------------------------------------------------------------------

template < typename SF, typename QD, typename PHYS >
class RDM_API WeakDirichlet::Term : public BcBase<SF,QD,PHYS> {

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

   // std::cout << "SF        ::type_name()    [" << SF::type_name()   << "]" << std::endl << std::flush;
   // std::cout << "SF        ::nb_nodes       [" << SF::nb_nodes      << "]" << std::endl << std::flush;
   // std::cout << "QD        ::nb_points      [" << QD::nb_points     << "]" << std::endl << std::flush;
   // std::cout << "PHYS      ::ndim           [" << PHYS::MODEL::_ndim        << "]" << std::endl << std::flush;
   // std::cout << "PHYS      ::neqs           [" << PHYS::MODEL::_neqs        << "]" << std::endl << std::flush;
   // std::cout << "PHYS      ::type_name()    [" << PHYS::type_name() << "]" << std::endl << std::flush;


   // temporary gradient of the shape functions in reference space
   typename SF::SF::GradientT grad_sf;
   // temporary values of shape functions in reference space
   typename SF::SF::ValueT value_sf;

   // initialize the interpolation matrix

   for(Uint q = 0; q < QD::nb_points; ++q)
     for(Uint n = 0; n < SF::nb_nodes; ++n)
     {
        SF::SF::compute_gradient( m_quadrature.coords.col(q), grad_sf  );
        SF::SF::compute_value   ( m_quadrature.coords.col(q), value_sf );

        Ni(q,n)     = value_sf[n];
        dNdKSI(q,n) = grad_sf[n];
     }

   vars.resize(DIM_3D);

   return_val.resize(PHYS::MODEL::_neqs );
 }

 /// Get the class name
 static std::string type_name () { return "WeakDirichlet.Term<" + SF::type_name() + "," + PHYS::type_name() + ">"; }

protected: // data

 /// valriables to pass to vectorial function
 std::vector<Real> vars;
 /// return value from the vectorial function
 RealVector return_val;

 /// helper object to compute the quadrature information
 const QD& m_quadrature;

 typedef Eigen::Matrix<Real, QD::nb_points, 1u>               WeightVT;

 typedef typename SF::NodesT                             NodeMT;

 typedef Eigen::Matrix<Real, QD::nb_points, SF::nb_nodes >     SFMatrixT;
 typedef Eigen::Matrix<Real, QD::nb_points, PHYS::MODEL::_ndim   >     QCoordMT;
 typedef Eigen::Matrix<Real, SF::nb_nodes , PHYS::MODEL::_neqs   >     SolutionMT;
 typedef Eigen::Matrix<Real, QD::nb_points, PHYS::MODEL::_neqs   >     QSolutionMT;
 typedef Eigen::Matrix<Real, PHYS::MODEL::_neqs,    PHYS::MODEL::_ndim   >     QSolutionVT;
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
  #if 0

    std::cout << "Face [" << B::idx() << "]" << std::endl;

    Uint face_idx = B::idx();

    mesh::FaceCellConnectivity& f2c =
        B::elements().get_child("cell_connectivity").as_type<mesh::FaceCellConnectivity>();

    // cf3_assert( f2c.is_bdry_face()[face_idx] );

    Handle< Component > neighbor_cells;
    Uint neighbor_cell_idx;

    common::Table<Uint>::ConstRow connected_cells = f2c.connectivity()[face_idx];
    Uint unified_neighbor_cell_idx = connected_cells[LEFT]; // boundary faces store idx on LEFT face

    // lookup the neighbor_cell elements and the index in its connectivity

    boost::tie(neighbor_cells,neighbor_cell_idx) = f2c.lookup().location( unified_neighbor_cell_idx );

    std::cout << "neighbor_cells [" << neighbor_cells->uri().string() << "]" << std::endl;

    common::Table<Uint>& connectivity =
        neighbor_cells->as_type<mesh::Elements>().geometry_space().connectivity();

    const common::Table<Uint>::ConstRow cell_nodes_idx = connectivity[ neighbor_cell_idx ];

    // prints the neighbor cell nodes idx

    std::cout << "cell_nodes_idx : ";
    for( Uint n = 0; n < cell_nodes_idx.size(); ++n )
    {
       std::cout << cell_nodes_idx[n] << " ";
    }
     std::cout << std::endl;

#endif

    // get face connectivity

     const mesh::Connectivity::ConstRow nodes_idx = (*B::connectivity)[B::idx()];

//   std::cout << "face_nodes_idx : ";
//   const Uint nbnodes = nodes_idx.shape()[1];
//   for( Uint n = 0; n < nbnodes; ++n )
//   {
//      std::cout << nodes_idx[n] << " ";
//   }
//   std::cout << std::endl;

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

   // ------------------------------------------------------
   /// @note Generic version

   for(Uint q=0; q < QD::nb_points; ++q)
   {
//    std::cout << "Gradient of face shape functions: " << std::endl;
//    std::cout << dNdKSI << std::endl;
//    std::cout << "nr. of rows = " << dNdKSI.rows() << std::endl;
//    std::cout << "nr. of cols = " << dNdKSI.cols() << std::endl;


    for(Uint dim = 0; dim < PHYS::MODEL::_ndim; ++dim)
    {
      dUdXq.col(dim) = dUdX[dim].row(q).transpose();
    }

    const Real jacob = std::sqrt( dX_q(q,XX)*dX_q(q,XX)+dX_q(q,YY)*dX_q(q,YY) );

    wj[q] = jacob * m_quadrature.weights[q];

    // compute the normal at quadrature point

    dN[XX] = -dX_q(q,YY)/jacob;
    dN[YY] =  dX_q(q,XX)/jacob;

//    std::cout << "n (generic) [" << nx << "," << ny << "]" << std::endl;

    // compute the flux F(u_h) and its correction F(u_g)

    PHYS::compute_properties(X_q.row(q),
                             U_q.row(q),
                             dUdXq,
                             B::phys_props);

    PHYS::flux(B::phys_props, Fu_h);

//     std::cout << "U_q =\n " << U_q.row(q) << std::endl;
//     std::cout << "Fu_h[XX] =" << std::endl;
//     std::cout <<  Fu_h.col(XX) << std::endl;
//     std::cout << "Fu_h[YY] =" << std::endl;
//     std::cout <<  Fu_h.col(XX) << std::endl;

    vars[XX] = X_q(q,XX);
    vars[YY] = X_q(q,YY);
    vars[ZZ] = 0.0;

    this->parent()->template handle<WeakDirichlet>()->function.evaluate(vars,return_val);

    PHYS::compute_properties(X_q.row(q),
                             return_val,
                             dUdXq,
                             B::phys_props);

    PHYS::flux(B::phys_props, Fu_g);

//     std::cout << "Prescribed value =\n " << return_val << std::endl;
//     std::cout << "Fu_g[XX] =" << std::endl;
//     std::cout <<  Fu_g.col(XX) << std::endl;
//     std::cout << "Fu_g[YY] =" << std::endl;
//     std::cout <<  Fu_g.col(XX) << std::endl;
//     std::cin.get();

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

//    std::cin.get();

//    std::cout << "Phi_n [" << Phi_n << "]" << std::endl;


   // ------------------------------------------------------

   // update the residual

   for (Uint n=0; n<SF::nb_nodes; ++n)
     for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
       (*B::residual)[nodes_idx[n]][v] += Phi_n(n,v);

//   if (B::idx() > 3) exit(0);
 }

}; // !WeakDirichlet::Term

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // cf3_RDM_WeakDirichlet_hpp
