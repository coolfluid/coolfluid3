// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP1_ElementTypes_hpp
#define cf3_mesh_LagrangeP1_ElementTypes_hpp

#include <boost/mpl/filter_view.hpp>
#include <boost/mpl/vector.hpp>

#include "mesh/ElementTypePredicates.hpp"

#include "mesh/LagrangeP1/Line1D.hpp"
#include "mesh/LagrangeP1/Line2D.hpp"
#include "mesh/LagrangeP1/Line3D.hpp"
#include "mesh/LagrangeP1/Quad2D.hpp"
#include "mesh/LagrangeP1/Quad3D.hpp"
#include "mesh/LagrangeP1/Triag2D.hpp"
#include "mesh/LagrangeP1/Triag3D.hpp"
#include "mesh/LagrangeP1/Hexa3D.hpp"
#include "mesh/LagrangeP1/Tetra3D.hpp"
#include "mesh/LagrangeP1/Prism3D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

///////////////////////////////////////////////////////////////////////////////


typedef boost::mpl::vector<
Line1D,
Line2D,
Line3D,
Quad2D,
Quad3D,
Triag2D,
Triag3D,
Hexa3D,
Tetra3D,
Prism3D
> ElementTypes;

typedef boost::mpl::filter_view<ElementTypes, IsCellType> CellTypes;
typedef boost::mpl::filter_view<ElementTypes, IsFaceType> FaceTypes;
typedef boost::mpl::filter_view<ElementTypes, IsEdgeType> EdgeTypes;

///////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP1_ElementTypes_hpp
