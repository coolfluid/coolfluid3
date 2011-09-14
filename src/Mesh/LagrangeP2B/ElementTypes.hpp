// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP2B_ElementTypes_hpp
#define CF_Mesh_LagrangeP2B_ElementTypes_hpp

#include <boost/mpl/filter_view.hpp>
#include <boost/mpl/vector.hpp>

#include "Mesh/ElementTypePredicates.hpp"

#include "Mesh/LagrangeP2B/Triag2D.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP2B {

///////////////////////////////////////////////////////////////////////////////

typedef boost::mpl::vector<
Triag2D
> ElementTypes;

typedef boost::mpl::filter_view<ElementTypes, IsCellType> CellTypes;
typedef boost::mpl::filter_view<ElementTypes, IsFaceType> FaceTypes;
typedef boost::mpl::filter_view<ElementTypes, IsEdgeType> EdgeTypes;

///////////////////////////////////////////////////////////////////////////////

} // LagrangeP2B
} // Mesh
} // CF

#endif // CF_Mesh_LagrangeP2B_ElementTypes_hpp
