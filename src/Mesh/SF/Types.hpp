// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Types_hpp
#define CF_Mesh_SF_Types_hpp

#include <boost/mpl/and.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/filter_view.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/vector.hpp>

#include "Line1DLagrangeP1.hpp"
#include "Line2DLagrangeP1.hpp"
#include "Line2DLagrangeP2.hpp"
#include "Line2DLagrangeP3.hpp"
#include "Line3DLagrangeP1.hpp"
#include "Triag2DLagrangeP1.hpp"
#include "Triag2DLagrangeP2.hpp"
#include "Triag2DLagrangeP3.hpp"
#include "Triag3DLagrangeP1.hpp"
#include "Quad2DLagrangeP1.hpp"
#include "Quad2DLagrangeP2.hpp"
#include "Quad3DLagrangeP1.hpp"
#include "Tetra3DLagrangeP1.hpp"
#include "Hexa3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

///////////////////////////////////////////////////////////////////////////////

/// List of all supported shapefunctions
typedef boost::mpl::vector< Line1DLagrangeP1,
                            Line2DLagrangeP1,
                            Line2DLagrangeP2,
                            Line2DLagrangeP3,
                            Line3DLagrangeP1,
                            Triag2DLagrangeP1,
                            Triag2DLagrangeP2,
                            Triag2DLagrangeP3,
                            Triag3DLagrangeP1,
                            Quad2DLagrangeP1,
                            Quad2DLagrangeP2,
                            Quad3DLagrangeP1,
                            Hexa3DLagrangeP1,
                            Tetra3DLagrangeP1
> Types;

///////////////////////////////////////////////////////////////////////////////

/// Compile-time predicate to determine if the given shape function represents a volume element, i.e. dimensions == dimensionality
struct IsVolumeElement
{
  template<typename ShapeFunctionT>
  struct apply
  {
    typedef typename boost::mpl::equal_to<boost::mpl::int_<ShapeFunctionT::dimension>,boost::mpl::int_<ShapeFunctionT::dimensionality> >::type type;
  };
};

/// Compile-time predicate to determine if the given shape function is compatible with the shape function given as template argument
/// i.e. when dimension, shape and number of nodes are equal
template<typename ShapeFunctionT>
struct IsCompatibleWith
{
  template<typename OtherShapeFunctionT>
  struct apply
  {
    typedef typename boost::mpl::and_
    <
      boost::mpl::equal_to
      <
        boost::mpl::int_<ShapeFunctionT::dimension>,
        boost::mpl::int_<OtherShapeFunctionT::dimension>
      >,
      boost::mpl::equal_to
      <
        boost::mpl::int_<ShapeFunctionT::shape>,
        boost::mpl::int_<OtherShapeFunctionT::shape>
      >
    >::type type;
  };
};

/// List of all supported shapefunctions for volume elements, 
typedef boost::mpl::filter_view<Types, IsVolumeElement> CellTypes;

///////////////////////////////////////////////////////////////////////////////

} // LagrangeSF
} // Mesh
} // CF

#endif // CF_Mesh_SF_Types_hpp
