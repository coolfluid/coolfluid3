// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Types_hpp
#define CF_Mesh_SF_Types_hpp

#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/filter_view.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/vector.hpp>

#include "Line1DLagrangeP1.hpp"
#include "Line2DLagrangeP1.hpp"
#include "Line3DLagrangeP1.hpp"
#include "Triag2DLagrangeP1.hpp"
#include "Triag3DLagrangeP1.hpp"
#include "Quad2DLagrangeP1.hpp"
#include "Quad3DLagrangeP1.hpp"
#include "Tetra3DLagrangeP1.hpp"
#include "Hexa3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

/// List of all supported shapefunctions
typedef boost::mpl::vector< Line1DLagrangeP1,
                            Line2DLagrangeP1,
                            Line3DLagrangeP1,
                            Triag2DLagrangeP1,
                            Triag3DLagrangeP1,
                            Quad2DLagrangeP1,
                            Quad3DLagrangeP1,
                            Hexa3DLagrangeP1,
                            Tetra3DLagrangeP1
> Types;

/// Compile-time predicate to determine if the given shape function represents a volume element, i.e. dimensions == dimensionality
struct IsVolumeElement
{
  template<typename ShapeFunctionT>
  struct apply
  {
    typedef typename boost::mpl::equal_to<boost::mpl::int_<ShapeFunctionT::dimension>,boost::mpl::int_<ShapeFunctionT::dimensionality> >::type type;
  };
};


/// List of all supported shapefunctions for volume elements, 
typedef boost::mpl::filter_view<Types, IsVolumeElement> VolumeTypes;

} // LagrangeSF
} // Mesh
} // CF

#endif // CF_Mesh_SF_Types_hpp
