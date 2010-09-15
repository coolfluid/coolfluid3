// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_Inverter4_hpp
#define CF_Math_Inverter4_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/MatrixInverter.hpp"
#include "Math/MatrixInverterT.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// Matrix inverter but using a abstract interface
/// @author Andrea Lani
/// @author Tiago Quintino
template < unsigned int SIZE >
struct InverterT : public MatrixInverter
{
  /// Invert the given matrix a and put the result in x
  /// @param a  matrix to invert
  /// @param x  result of the matrix inversion
  virtual void invert(const RealMatrix& a, RealMatrix& x)  {  m_inverter.invert(a,x);  }

private:

  /// the actual inverter object
  MatrixInverterT<SIZE> m_inverter;

}; // end of class Inverter4

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_Inverter4_hpp
