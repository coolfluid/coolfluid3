// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Integrators_Gauss_hpp
#define cf3_mesh_Integrators_Gauss_hpp

#include <boost/mpl/for_each.hpp>
#include <boost/foreach.hpp>

#include "common/BasicExceptions.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "common/Table.hpp"

#include "mesh/Integrators/GaussImplementation.hpp"

namespace cf3 {
namespace mesh {

/// @brief Functions to provide integration over elements
namespace Integrators {

/// Integral of a functor over an element, using the template-supplied order and element shape
/// @param functor Functor to be evaluated. Must provide operator() without any arguments and return a result compatible with ResultT
/// @param mapped_coords Appropriately sized storage that will be set to the current mapped coordinates before each
/// functor evaluation
/// @param result Appropriately sized and typed result of the integration
template<Uint Order, GeoShape::Type Shape, typename FunctorT, typename MappedCoordsT, typename ResultT>
void gauss_integrate(const FunctorT& functor, MappedCoordsT& mapped_coords, ResultT& result)
{
  GaussIntegrator<Order, Shape>::integrate(functor, mapped_coords, result);
}

} // Integrators
} // mesh
} // cf3

#endif /* CF3_Mesh_Integrators_Gauss_hpp */
