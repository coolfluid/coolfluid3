// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_math_MatrixTypes_hpp
#define cf3_math_MatrixTypes_hpp

#include "common/EigenAssertions.hpp"
#include <Eigen/Dense>
#include <Eigen/StdVector>

#include "common/CF.hpp"

namespace cf3 {

//////////////////////////////////////////////////////////////////////////////////////////////

/// Dynamic sized matrix of Real scalars
typedef Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic> RealMatrix;

/// Dynamic sized column vector
typedef Eigen::Matrix<Real, Eigen::Dynamic, 1>    RealVector;

/// Dynamic sized column vector (alternative naming)
typedef RealVector                                RealColVector;

/// Dynamic sized row vector
typedef Eigen::Matrix<Real, 1, Eigen::Dynamic>    RealRowVector;

// Fixed size matrix typedefs for 2x2, 3x3 and 4x4 matrices
typedef Eigen::Matrix<Real, 2, 2> RealMatrix2;     ///< Fixed size 2x2 matrix
typedef Eigen::Matrix<Real, 3, 3> RealMatrix3;     ///< Fixed size 3x3 matrix
typedef Eigen::Matrix<Real, 4, 4> RealMatrix4;     ///< Fixed size 4x4 matrix

// Fixed size vectors for 1, 2, 3 and 4 elements
typedef Eigen::Matrix<Real, 1, 1> RealVector1;     ///< Fixed size 1x1 column vector
typedef Eigen::Matrix<Real, 2, 1> RealVector2;     ///< Fixed size 2x1 column vector
typedef Eigen::Matrix<Real, 3, 1> RealVector3;     ///< Fixed size 3x1 column vector
typedef Eigen::Matrix<Real, 4, 1> RealVector4;     ///< Fixed size 4x1 column vector

// Fixed size vectors for 1, 2, 3 and 4 elements
typedef Eigen::Matrix<Real, 1, 1> RealRowVector1;  ///< Fixed size 1x1 row vector
typedef Eigen::Matrix<Real, 1, 2> RealRowVector2;  ///< Fixed size 1x2 row vector
typedef Eigen::Matrix<Real, 1, 3> RealRowVector3;  ///< Fixed size 1x3 row vector
typedef Eigen::Matrix<Real, 1, 4> RealRowVector4;  ///< Fixed size 1x4 row vector

//////////////////////////////////////////////////////////////////////////////////////////////

} // cf3

#endif // cf3_math_MatrixTypes_hpp
