// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/algorithm/string/erase.hpp>
#include <boost/tuple/tuple.hpp>

#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Link.hpp"

#include "math/Consts.hpp"
#include "math/MatrixTypesConversion.hpp"

#include "mesh/ShapeFunctionInterpolation.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Field.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/UnifiedData.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  using namespace common;
  using namespace math::Consts;

////////////////////////////////////////////////////////////////////////////////

cf3::common::ComponentBuilder < ShapeFunctionInterpolation, InterpolationFunction, LibMesh > ShapeFunctionInterpolation_builder;

////////////////////////////////////////////////////////////////////////////////

void ShapeFunctionInterpolation::compute_interpolation_weights(const RealVector& coordinate, const std::vector<SpaceElem>& stencil,
                                           std::vector<Uint>& source_field_points, std::vector<Real>& source_field_weights)
{
  cf3_assert_desc("Dictionary not configured in "+uri().string(), is_not_null(m_dict) );

  if (stencil.size()>1)
    throw SetupError(FromHere(),"The stencil for this interpolation function should be the centre cell itself");

  const SpaceElem& element = stencil[0];

  RealMatrix element_coords = element.comp->support().geometry_space().get_coordinates(element.idx);
  RealVector mapped_coord   = element.comp->support().element_type().mapped_coordinate(coordinate,element_coords);
  RealVector sf_values = element.shape_function().value(mapped_coord);
  source_field_points.resize(element.shape_function().nb_nodes());
  source_field_weights.resize(source_field_points.size());
  for (Uint n=0; n<source_field_points.size(); ++n)
  {
    source_field_points[n] = element.nodes()[n];
    source_field_weights[n] = sf_values[n];
  }
}

//////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
