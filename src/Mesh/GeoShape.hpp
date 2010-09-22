// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_GeoShape_hpp
#define CF_Mesh_GeoShape_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/LibMesh.hpp"
#include "Common/EnumT.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh{

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

  typedef Common::EnumT< GeoShape > ConverterBase;

  struct Mesh_API Convert : public ConverterBase
  {
    /// storage of the enum forward map
    static ConverterBase::FwdMap_t all_fwd;
    /// storage of the enum reverse map
    static ConverterBase::BwdMap_t all_rev;
  };

}; // class GeoShape

////////////////////////////////////////////////////////////////////////////////

Mesh_API std::ostream& operator<< ( std::ostream& os, const GeoShape::Type& in );
Mesh_API std::istream& operator>> ( std::istream& is, GeoShape::Type& in );

////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_GeoShape_hpp
