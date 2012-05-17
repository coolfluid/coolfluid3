// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_InterpolationFunction_hpp
#define cf3_mesh_InterpolationFunction_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/LibMesh.hpp"
#include "mesh/Space.hpp"

namespace cf3 {
namespace mesh {
  
  class Dictionary;

////////////////////////////////////////////////////////////////////////////////

class Mesh_API InterpolationFunction : public common::Component
{
public:

  /// @brief Constructor
  InterpolationFunction( const std::string& name );

  /// @brief Type name
  static std::string type_name() { return "InterpolationFunction"; }


  /// @brief Compute interpolation points and weights for points contained in a given stencil for a given coordinate
  virtual void compute_interpolation_weights(const RealVector& coordinate, const std::vector<SpaceElem>& stencil,
                                             std::vector<Uint>& source_field_points, std::vector<Real>& source_field_weights) = 0;

protected:

  Handle<Dictionary> m_dict;
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_InterpolationFunction_hpp
