// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_Types_hpp
#define CF_Math_Types_hpp

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  /// Enumeration of the dimensions
  enum Dim                  { DIM_0D, DIM_1D, DIM_2D, DIM_3D };
  /// Enumeration of the coordinates indexes
  enum CoordXYZ             { XX, YY, ZZ };
  /// Enumeration of the reference coordinates indexes
  enum CoordRef             { KSI, ETA, ZTA };
  /// Enumeration of sides
  enum Side                 { LEFT, RIGHT };

} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_Types_hpp
