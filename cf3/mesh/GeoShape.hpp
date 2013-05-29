// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_GeoShape_hpp
#define cf3_mesh_GeoShape_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/LibMesh.hpp"
#include "common/EnumT.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh{

////////////////////////////////////////////////////////////////////////////////

class Mesh_API GeoShape
{
  public:

  /// Enumeration of the Shapes recognized in CF
  enum Type  { INVALID = -1,
               POINT   = 0,
               LINE    = 1,
               TRIAG   = 2,
               QUAD    = 3,
               TETRA   = 4,
               PYRAM   = 5,
               PRISM   = 6,
               HEXA    = 7  };

  typedef common::EnumT< GeoShape > ConverterBase;

  struct Mesh_API Convert : public ConverterBase
  {
    /// constructor where all the converting maps are built
    Convert();
    /// get the unique instance of the converter class
    static Convert& instance();
  };

}; // class GeoShape

////////////////////////////////////////////////////////////////////////////////

Mesh_API std::ostream& operator<< ( std::ostream& os, const GeoShape::Type& in );
Mesh_API std::istream& operator>> ( std::istream& is, GeoShape::Type& in );

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_GeoShape_hpp
