// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP0_ElementTypes_hpp
#define cf3_mesh_LagrangeP0_ElementTypes_hpp

#include <boost/mpl/filter_view.hpp>
#include <boost/mpl/vector.hpp>

#include "mesh/ElementTypePredicates.hpp"

#include "mesh/LagrangeP0/Point1D.hpp"
#include "mesh/LagrangeP0/Point2D.hpp"
#include "mesh/LagrangeP0/Point3D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP0 {

///////////////////////////////////////////////////////////////////////////////

typedef boost::mpl::vector<
Point1D,
Point2D,
Point3D
> ElementTypes;

typedef boost::mpl::filter_view<ElementTypes, IsCellType> CellTypes;
typedef boost::mpl::filter_view<ElementTypes, IsFaceType> FaceTypes;
typedef boost::mpl::filter_view<ElementTypes, IsEdgeType> EdgeTypes;

///////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP0_ElementTypes_hpp
