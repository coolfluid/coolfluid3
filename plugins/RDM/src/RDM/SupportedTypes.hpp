// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_SupportedTypes_hpp
#define CF_RDM_SupportedTypes_hpp

#include <boost/mpl/vector.hpp>

#include "Mesh/CElements.hpp"

#include "Mesh/SF/Triag2DLagrangeP1.hpp"
#include "Mesh/SF/Triag2DLagrangeP2.hpp"
#include "Mesh/SF/Triag2DLagrangeP2B.hpp"
#include "Mesh/SF/Triag2DLagrangeP3.hpp"

#include "Mesh/SF/Quad2DLagrangeP1.hpp"
#include "Mesh/SF/Quad2DLagrangeP2.hpp"

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

/// List of all supported shapefunctions
typedef boost::mpl::vector<
Triag2DLagrangeP1,
Triag2DLagrangeP2,
Triag2DLagrangeP2B,
Triag2DLagrangeP3,
Quad2DLagrangeP1,
Quad2DLagrangeP2,
> CellTypes;

/// Predicate class to test if the region contains a specific element type
template < typename TYPE >
struct IsElementType
{
  bool operator()(const Mesh::CElements& component)
  {
    return Mesh::IsElementType<TYPE>()( component.element_type() );
  }
};

///////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_SupportedTypes_hpp
