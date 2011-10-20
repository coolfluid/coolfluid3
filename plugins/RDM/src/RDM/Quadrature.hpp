// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_Quadrature_hpp
#define cf3_RDM_Quadrature_hpp

#include "Mesh/CElements.hpp"

#include "Mesh/Integrators/GaussImplementation.hpp"

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

/// function returning positive number or zero
inline Real plus ( Real x )
{
  return std::max( 0. , x );
}

/// function returning negative number or zero
inline Real minus ( Real x )
{
  return std::min( 0. , x );
}


/// Predicate class to test if the region contains a specific element type
template < typename TYPE >
struct IsElementType
{
  bool operator()(const Mesh::CElements& component)
  {
    return Mesh::IsElementType<TYPE>()( component.element_type() );
  }
};


/// Define the default quadrature for each element type
template < typename SF, Uint order = SF::order >
struct DefaultQuadrature
{
  typedef Mesh::Integrators::GaussMappedCoords< order, SF::shape> type;
};


////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // cf3_RDM_SupportedCells_hpp
