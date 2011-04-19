// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_CSysFluxOp2D_hpp
#define CF_RDM_CSysFluxOp2D_hpp

#include <iostream>

#include <Eigen/Dense>


/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
class RDM_SCHEMES_API CSysFluxOp2D
{
public: // typedefs



public: // functions
  /// Contructor
  /// @param name of the component
  CSysFluxOp2D ( );

  /// Virtual destructor
  virtual ~CSysFluxOp2D() {};

  /// Get the class name
  static std::string type_name () { return "CSysFluxOp2D<" + SHAPEFUNC::type_name() + "," + QUADRATURE::type_name() +  ">"; }

  /// Compute the operator values at all quadrature nodes
  void compute(const NodeMT& nodes,
               const SolutionMT& solution,
               PhysicsMT    Kiq[],
               PhysicsVT    LU[],
               PhysicsMT    DvPlus[],
               Eigen::Matrix<Real, QUADRATURE::nb_points, 1u>& wj);
    
protected: // data



};


template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
CSysFluxOp2D<SHAPEFUNC,QUADRATURE,PHYSICS>::CSysFluxOp2D() : m_quadrature( QUADRATURE::instance() )
{

}



template < typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
void CSysFluxOp2D<SHAPEFUNC,QUADRATURE,PHYSICS>::compute(const NodeMT& nodes,
                                                         const SolutionMT& solution,
                                                         PhysicsMT   Kiq[],
                                                         PhysicsVT   LU[],
                                                         PhysicsMT   DvPlus[],
                                                         Eigen::Matrix<Real, QUADRATURE::nb_points, 1u>& wj)
{


}


////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_CSysFluxOp2D_hpp
