// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_PointInterpolator_hpp
#define cf3_mesh_PointInterpolator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/LibMesh.hpp"
#include "mesh/Space.hpp"

#include "mesh/Field.hpp"

namespace cf3 {
namespace mesh {

  class Dictionary;
  class Field;
  class ElementFinder;
  class StencilComputer;
  class InterpolationFunction;

////////////////////////////////////////////////////////////////////////////////

/// @brief PointInterpolator to interpolate a field to a point
/// @author Willem Deconinck
class Mesh_API APointInterpolator : public common::Component {
public: // functions

  /// Contructor
  /// @param name of the component
  APointInterpolator ( const std::string& name );

  /// Virtual destructor
  virtual ~APointInterpolator() {}

  /// Get the class name
  static std::string type_name () { return "APointInterpolator"; }

  // --------- Signals ---------

  void signal_interpolate( common::SignalArgs& node  );
  
  // --------- Direct access ---------

  template<typename VectorT>
  bool interpolate(const Field& field, const RealVector& coordinate, VectorT& interpolated_value);

  virtual bool compute_storage(const RealVector& coordinate, SpaceElem& element, std::vector<SpaceElem>& stencil, std::vector<Uint>& points, std::vector<Real>& weights) = 0;

private: // functions

  void configure_dict();

protected: // data
  
  /// source dictionary
  Handle<Dictionary> m_dict;

  /// Temporary variables to avoid allocation, for use in
  /// interpolate(coordinate,interpolated_value)
  SpaceElem m_element;
  std::vector<SpaceElem> m_stencil;
  std::vector<Uint> m_source_field_points;
  std::vector<Real> m_source_field_weights;

};

////////////////////////////////////////////////////////////////////////////////

template <typename VectorT>
bool APointInterpolator::interpolate(const Field& field, const RealVector& coordinate, VectorT& interpolated_value)
{
  // 1) Compute all useful information for interpolation
  const bool element_found = compute_storage(coordinate,m_element,m_stencil,m_source_field_points,m_source_field_weights);
  if (!element_found)
  {
    // Not interpolated because coordinate not found with element finder
    return false;
  }

  // 2) Interpolate

  for (Uint v=0; v<interpolated_value.size(); ++v)
  {
    interpolated_value[v]=0.;
    for (Uint i=0; i<m_source_field_points.size(); ++i)
    {
      interpolated_value[v] += field[m_source_field_points[i]][v] * m_source_field_weights[i];
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

/// @brief A general configurable point interpolator
class Mesh_API PointInterpolator : public APointInterpolator {
public: // functions

  /// Contructor
  /// @param name of the component
  PointInterpolator ( const std::string& name );

  /// Virtual destructor
  virtual ~PointInterpolator() {}

  /// Get the class name
  static std::string type_name () { return "PointInterpolator"; }

  // --------- Direct access ---------

  virtual bool compute_storage(const RealVector& coordinate, SpaceElem& element, std::vector<SpaceElem>& stencil, std::vector<Uint>& points, std::vector<Real>& weights);

private: // functions

  void configure_element_finder();

  void configure_stencil_computer();

  void configure_interpolator_function();

private: // data

  /// The strategy to find the element
  Handle<ElementFinder>          m_element_finder;

  /// The strategy to compute the stencil
  Handle<StencilComputer>        m_stencil_computer;

  /// The strategy to interpolate.
  Handle<InterpolationFunction>  m_interpolator_function;
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_PointInterpolator_hpp
