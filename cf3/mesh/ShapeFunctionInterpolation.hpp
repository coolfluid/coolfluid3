// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_ShapeFunctionInterpolation_hpp
#define cf3_mesh_ShapeFunctionInterpolation_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/BoostArray.hpp"
#include "mesh/Interpolator.hpp"
#include "mesh/InterpolationFunction.hpp"
#include "mesh/Elements.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

class Mesh_API ShapeFunctionInterpolation : public InterpolationFunction
{
public:
  /// constructor
  ShapeFunctionInterpolation( const std::string& name ) : InterpolationFunction(name) {}

  /// Gets the Class name
  static std::string type_name() { return "ShapeFunctionInterpolation"; }


  virtual void compute_interpolation_weights(const RealVector& coordinate, const std::vector<SpaceElem>& stencil,
                                             std::vector<Uint>& source_field_points, std::vector<Real>& source_field_weights);
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_neu_ShapeFunctionInterpolation_hpp
