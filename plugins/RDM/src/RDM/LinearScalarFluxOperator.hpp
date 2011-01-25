// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_LinearScalarFluxOperator_hpp
#define CF_Solver_LinearScalarFluxOperator_hpp

#include <Eigen/Dense>

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE>
class RDM_API LinearScalarFluxOperator
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<LinearScalarFluxOperator> Ptr;
  typedef boost::shared_ptr<LinearScalarFluxOperator const> ConstPtr;

  /// output matrix types
  typedef Eigen::Matrix<Real, QUADRATURE::nb_points, SHAPEFUNC::nb_nodes> SFMatrixT;

public: // functions
  /// Contructor
  /// @param name of the component
  LinearScalarFluxOperator ( );

  /// Virtual destructor
  virtual ~LinearScalarFluxOperator() {};

  /// Get the class name
  static std::string type_name () { return "LinearScalarFluxOperator<" + SHAPEFUNC::type_name() + ">"; }

  /// Compute the operator values at all quadrature nodes
  void compute(const typename SHAPEFUNC::NodeMatrixT & nodes, const RealVector& solution, SFMatrixT& sf_oper_values, RealVector& flux_oper_values);
    
private: // data

  const QUADRATURE& m_quadrature;

  typename SHAPEFUNC::MappedGradientT m_mapped_grad; //Gradient of the shape functions in reference space
  typename SHAPEFUNC::ShapeFunctionsT m_shapefunc;     //Values of shape functions in reference space
  typename SHAPEFUNC::CoordsT m_grad_solution;
  typename SHAPEFUNC::CoordsT m_grad_x;
  typename SHAPEFUNC::CoordsT m_grad_y;
};


template<typename SHAPEFUNC, typename QUADRATURE>
LinearScalarFluxOperator<SHAPEFUNC,QUADRATURE>::LinearScalarFluxOperator() : m_quadrature( QUADRATURE::instance() )
{
}



template<typename SHAPEFUNC, typename QUADRATURE>
void LinearScalarFluxOperator<SHAPEFUNC,QUADRATURE>::compute(const typename SHAPEFUNC::NodeMatrixT & nodes,
                                                             const RealVector& solution,
							     SFMatrixT& sf_oper_values,
                                                             RealVector& flux_oper_values)
{

  //Loop over quadrature points
  for (Uint q=0; q<QUADRATURE::nb_points; ++q)
  {
    SHAPEFUNC::mapped_gradient( m_quadrature.coords.col(q), m_mapped_grad );
    SHAPEFUNC::shape_function ( m_quadrature.coords.col(q), m_shapefunc   );

    Real x=0;
    Real y=0;
    Real u=0;

    ///Compute solution value and coordinates of quadrature point in physical space
    for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    {
      x += m_shapefunc[n] * nodes(n, XX);
      y += m_shapefunc[n] * nodes(n, YY);
      u += m_shapefunc[n] * solution[n];
    }

    m_grad_x.setZero();
    m_grad_y.setZero();

    // Compute the components of the Jacobian matrix representing the transformation
    // physical -> reference space
    for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    {
      m_grad_x[XX] += m_mapped_grad(XX,n) * nodes(n, XX);
      m_grad_x[YY] += m_mapped_grad(YY,n) * nodes(n, XX);
      m_grad_y[XX] += m_mapped_grad(XX,n) * nodes(n, YY);
      m_grad_y[YY] += m_mapped_grad(YY,n) * nodes(n, YY);
    }

    const Real jacobian = m_grad_x[XX]*m_grad_y[YY]-m_grad_x[YY]*m_grad_y[XX];

    //Compute the gradient of the solution in physical space
    m_grad_solution.setZero();

    for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    {
      const Real dNdx = 1.0/jacobian * (  m_grad_y[YY]*m_mapped_grad(XX,n) - m_grad_y[XX]*m_mapped_grad(YY,n) );
      const Real dNdy = 1.0/jacobian * ( -m_grad_x[YY]*m_mapped_grad(XX,n) + m_grad_x[XX]*m_mapped_grad(YY,n) );

      m_grad_solution[XX] += dNdx*solution[n];
      m_grad_solution[YY] += dNdy*solution[n];

      sf_oper_values(q,n) = y * dNdx - x * dNdy;
    }

   //Store the value of the operator acting on the flux at this quadrature point. The value is directly
   //multiplied by quadrature weight and jacobian:
   flux_oper_values[q] = (y * m_grad_solution[XX] - x * m_grad_solution[YY]) * jacobian * m_quadrature.weights[q];

  } // Loop over quadrature nodes

}


////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_CSchemeLDAT_hpp
