// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP3_ElementTypes_hpp
#define cf3_mesh_LagrangeP3_ElementTypes_hpp

#include <boost/mpl/filter_view.hpp>
#include <boost/mpl/vector.hpp>

#include "mesh/ElementTypePredicates.hpp"

#include "mesh/LagrangeP3/Line2D.hpp"
#include "mesh/LagrangeP3/Quad2D.hpp"
#include "mesh/LagrangeP3/Triag2D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP3 {

///////////////////////////////////////////////////////////////////////////////

typedef boost::mpl::vector<
Line2D,
Quad2D,
Triag2D
> ElementTypes;

typedef boost::mpl::filter_view<ElementTypes, IsCellType> CellTypes;
typedef boost::mpl::filter_view<ElementTypes, IsFaceType> FaceTypes;
typedef boost::mpl::filter_view<ElementTypes, IsEdgeType> EdgeTypes;


///////////////////////////////////////////////////////////////////////////////

} // LagrangeP3
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP3_ElementTypes_hpp
