// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_MatrixTypes_hpp
#define cf3_physics_MatrixTypes_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"

namespace cf3 {
namespace physics {

  template < Uint NDIM, Uint NEQS=0, Uint NVAR=0, Uint NGRAD=0 >
  struct MatrixTypes
  {
    typedef Eigen::Matrix<Real,NDIM,1>     ColVector_NDIM;
    typedef Eigen::Matrix<Real,1,NEQS>     RowVector_NEQS;
    typedef Eigen::Matrix<Real,1,NVAR>     RowVector_NVAR;
    typedef Eigen::Matrix<Real,1,NGRAD>    RowVector_NGRAD;
    typedef Eigen::Matrix<Real,NDIM,NDIM>  Matrix_NDIMxNDIM;
    typedef Eigen::Matrix<Real,NDIM,NEQS>  Matrix_NDIMxNEQS;
    typedef Eigen::Matrix<Real,NEQS,NEQS>  Matrix_NEQSxNEQS;
    typedef Eigen::Matrix<Real,NDIM,NVAR>  Matrix_NDIMxNVAR;
    typedef Eigen::Matrix<Real,NDIM,NGRAD> Matrix_NDIMxNGRAD;
  };

////////////////////////////////////////////////////////////////////////////////

} // physics
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_physics_MatrixTypes_hpp
