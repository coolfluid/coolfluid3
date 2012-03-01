// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_math_MatrixTypesConversion_hpp
#define cf3_math_MatrixTypesConversion_hpp

#include "common/BoostArray.hpp"

#include "math/MatrixTypes.hpp"

namespace cf3 {
namespace math {

//////////////////////////////////////////////////////////////////////////////////////////////

/// @name Conversions to std::vector<Real>
//@{

/// @brief Copy std::vector to dynamic RealMatrix types
void copy(const std::vector<Real>& vector, RealMatrix& realmatrix);

/// @brief Copy dynamic RealMatrix types to std::vector
void copy(const RealMatrix& realmatrix, std::vector<Real>& vector);

/// @brief Copy std::vector to static RealMatrix types
template <Uint ROWS, Uint COLS>
void copy(const std::vector<Real>& vector, Eigen::Matrix<Real, ROWS, COLS>& realmatrix);

/// @brief Copy static RealMatrix types to std::vector
template <Uint ROWS, Uint COLS>
void copy(const Eigen::Matrix<Real, ROWS, COLS>& realmatrix, std::vector<Real>& vector);

/// @brief Copy std::vector to dynamic RealVector
void copy(const std::vector<Real>& vector, RealVector& realvector);

/// @brief Copy dynamic RealVector to std::vector
void copy(const RealVector& realvector, std::vector<Real>& vector);

/// @brief Copy std::vector to static RealVector types
template <Uint ROWS>
void copy(const std::vector<Real>& vector, Eigen::Matrix<Real, ROWS, 1>& realvector);

/// @brief Copy static RealVector types to std::vector
template <Uint ROWS>
void copy(const Eigen::Matrix<Real, ROWS, 1>& realvector, std::vector<Real>& vector);

/// @brief Copy std::vector to dynamic RealVector
void copy(const std::vector<Real>& vector, RealRowVector& realrowvector);

/// @brief Copy dynamic RealVector to std::vector
void copy(const RealRowVector& realrowvector, std::vector<Real>& vector);

/// @brief Copy std::vector to static RealVector types
template <Uint COLS>
void copy(const std::vector<Real>& vector, Eigen::Matrix<Real, 1, COLS>& realrowvector);

/// @brief Copy static RealVector types to std::vector
template <Uint COLS>
void copy(const Eigen::Matrix<Real, 1, COLS>& realrowvector, std::vector<Real>& vector);

/// @brief Copy std::vector to dynamic RealMatrix types
void copy(const std::vector<Real>& vector, RealMatrix& realmatrix);

/// @brief Copy dynamic RealMatrix types to std::vector
void copy(const RealMatrix& realmatrix, std::vector<Real>& vector);
//@}


/// @name Conversions to boost::multi_array<Real,2> rows
//@{
/// @brief shortcut typedef
typedef boost::detail::multi_array::sub_array<Real,1> boost_row_t;
/// @brief shortcut typedef
typedef const boost::detail::multi_array::const_sub_array<Real,1> boost_constrow_t;

/// @brief Copy boost_constrow_t to dynamic RealVector types
void copy(const boost_constrow_t& vector, RealVector& realvector);

/// @brief Copy dynamic RealVector types to boost_row_t
void copy(const RealVector& realvector, boost_row_t& vector);

//////////////////////////////////////////////////////////////////////////////////////////////

inline void copy(const std::vector<Real>& vector, RealMatrix& realmatrix)
{
  cf3_assert(realmatrix.size() == vector.size());
  for (Uint row=0; row<realmatrix.rows(); ++row)
  {
    for (Uint col=0; col<realmatrix.cols(); ++col)
    {
      realmatrix(row,col) = vector[realmatrix.cols()*row + col];
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////

inline void copy(const RealMatrix& realmatrix, std::vector<Real>& vector)
{
  cf3_assert(vector.size() == realmatrix.size());
  for (Uint row=0; row<realmatrix.rows(); ++row)
  {
    for (Uint col=0; col<realmatrix.cols(); ++col)
    {
      vector[realmatrix.cols()*row+col] = realmatrix(row,col);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////

template <Uint ROWS, Uint COLS>
inline void copy(const std::vector<Real>& vector, Eigen::Matrix<Real, ROWS, COLS>& realmatrix)
{
  cf3_assert(realmatrix.size() == vector.size());
  for (Uint row=0; row<ROWS; ++row)
  {
    for (Uint col=0; col<COLS; ++col)
    {
      realmatrix(row,col) = vector[COLS*row + col];
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////

template <Uint ROWS, Uint COLS>
inline void copy(const Eigen::Matrix<Real, ROWS, COLS>& realmatrix, std::vector<Real>& vector)
{
  cf3_assert(vector.size() == realmatrix.size());
  for (Uint row=0; row<ROWS; ++row)
  {
    for (Uint col=0; col<COLS; ++col)
    {
      vector[COLS*row+col] = realmatrix(row,col);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////

inline void copy(const std::vector<Real>& vector, RealVector& realvector)
{
  cf3_assert(realvector.size() == vector.size());
  for (Uint row=0; row<realvector.rows(); ++row)
  {
    realvector[row] = vector[row];
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////

inline void copy(const RealVector& realvector, std::vector<Real>& vector)
{
  cf3_assert(vector.size() == realvector.size());
  for (Uint row=0; row<realvector.rows(); ++row)
  {
    vector[row] = realvector[row];
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////

template <Uint ROWS>
inline void copy(const std::vector<Real>& vector, Eigen::Matrix<Real, ROWS, 1>& realvector)
{
  cf3_assert(realvector.size() == vector.size());
  for (Uint row=0; row<ROWS; ++row)
  {
    realvector[row] = vector[row];
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////

template <Uint ROWS>
inline void copy(const Eigen::Matrix<Real, ROWS, 1>& realvector, std::vector<Real>& vector)
{
  cf3_assert(vector.size() == realvector.size());
  for (Uint row=0; row<ROWS; ++row)
  {
      vector[row] = realvector[row];
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////

inline void copy(const boost_constrow_t& vector, RealVector& realvector)
{
  cf3_assert(realvector.size() == vector.size());
  for (Uint row=0; row<realvector.rows(); ++row)
  {
    realvector[row] = vector[row];
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////

inline void copy(const RealVector& realvector, boost_row_t& vector)
{
  cf3_assert(vector.size() == realvector.size());
  for (Uint row=0; row<realvector.rows(); ++row)
  {
    vector[row] = realvector[row];
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////

} // math
} // cf3

#endif // cf3_Math_MatrixTypesConversion_hpp
