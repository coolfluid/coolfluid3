// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_PhysDataBase_hpp
#define cf3_sdm_PhysDataBase_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

//////////////////////////////////////////////////////////////////////////////

template <Uint NB_EQS, Uint NB_DIM>
struct PhysDataBase
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  enum { NDIM = NB_DIM }; ///< number of dimensions
  enum { NEQS = NB_EQS }; ///< number of independent variables or equations
  typedef Eigen::Matrix<Real,NDIM,1> RealVectorNDIM;
  typedef Eigen::Matrix<Real,NEQS,1> RealVectorNEQS;

  RealVectorNEQS solution;
  RealVectorNDIM coord;
};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_PhysDataBase_hpp
