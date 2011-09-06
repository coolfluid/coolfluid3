// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

namespace CF {
namespace Mesh {
namespace SF {

///////////////////////////////////////////////////////////////////////////////

/// List of all supported shapefunctions
typedef boost::mpl::vector<

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
