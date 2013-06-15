// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_Defs_hpp
#define cf3_Math_Defs_hpp

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

  /// Enumeration of the dimensions
  enum Dim                  { DIM_0D, DIM_1D, DIM_2D, DIM_3D };
  /// Enumeration of the coordinates indexes
  enum CoordXYZ             { XX, YY, ZZ };
  /// Enumeration of the reference coordinates indexes
  enum CoordRef             { KSI, ETA, ZTA };
  /// Enumeration of sides
  enum Side                 { LEFT, RIGHT };
  /// Variable types
  enum VarType              { ARRAY=0, SCALAR=1, VECTOR_2D=2, VECTOR_3D=3, TENSOR_2D=4, TENSOR_3D=9};

} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Math_Defs_hpp
