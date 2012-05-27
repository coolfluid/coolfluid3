// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_PointInterpolatorT_hpp
#define cf3_mesh_PointInterpolatorT_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "mesh/PointInterpolator.hpp"

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// @brief A compile-time configured point interpolator, making sure that element-finder,
/// stencil computer, and interpolation function match together
template< typename ELEMENTFINDER, typename STENCILCOMPUTER, typename INTERPOLATIONFUNCTION>
class Mesh_API PointInterpolatorT : public APointInterpolator {
public: // functions

  /// Contructor
  /// @param name of the component
  PointInterpolatorT ( const std::string& name );

  /// Virtual destructor
  virtual ~PointInterpolatorT() {}

  /// Get the class name
  static std::string type_name ();

  // --------- Direct access ---------

  virtual bool compute_storage(const RealVector& coordinate, SpaceElem& element, std::vector<SpaceElem>& stencil, std::vector<Uint>& points, std::vector<Real>& weights);

private: // functions

  void configure();

private: // data

  /// The strategy to find the element
  Handle<ELEMENTFINDER>          m_element_finder;

  /// The strategy to compute the stencil
  Handle<STENCILCOMPUTER>        m_stencil_computer;

  /// The strategy to interpolate.
  Handle<INTERPOLATIONFUNCTION>  m_interpolator_function;
};

////////////////////////////////////////////////////////////////////////////////

template< typename ELEMENTFINDER, typename STENCILCOMPUTER, typename INTERPOLATIONFUNCTION>
std::string PointInterpolatorT<ELEMENTFINDER,STENCILCOMPUTER,INTERPOLATIONFUNCTION>::type_name()
{
  return "PointInterpolatorT<"+ELEMENTFINDER::type_name()+","+STENCILCOMPUTER::type_name()+","+INTERPOLATIONFUNCTION::type_name()+">";
}

////////////////////////////////////////////////////////////////////////////////

template< typename ELEMENTFINDER, typename STENCILCOMPUTER, typename INTERPOLATIONFUNCTION>
PointInterpolatorT<ELEMENTFINDER,STENCILCOMPUTER,INTERPOLATIONFUNCTION>::PointInterpolatorT ( const std::string& name ) :
  APointInterpolator(name)
{
  m_element_finder = create_static_component<ELEMENTFINDER>("element_finder");
  m_stencil_computer = create_static_component<STENCILCOMPUTER>("stencil_computer");
  m_interpolator_function = create_static_component<INTERPOLATIONFUNCTION>("function");

  options().option("dict").attach_trigger( boost::bind( &PointInterpolatorT<ELEMENTFINDER,STENCILCOMPUTER,INTERPOLATIONFUNCTION>::configure, this) );
}

////////////////////////////////////////////////////////////////////////////////

template< typename ELEMENTFINDER, typename STENCILCOMPUTER, typename INTERPOLATIONFUNCTION>
void PointInterpolatorT<ELEMENTFINDER,STENCILCOMPUTER,INTERPOLATIONFUNCTION>::configure()
{
  m_element_finder->options().set("dict",m_dict);
  m_stencil_computer->options().set("dict",m_dict);
  m_interpolator_function->options().set("dict",m_dict);
}

////////////////////////////////////////////////////////////////////////////////

template< typename ELEMENTFINDER, typename STENCILCOMPUTER, typename INTERPOLATIONFUNCTION>
bool PointInterpolatorT<ELEMENTFINDER,STENCILCOMPUTER,INTERPOLATIONFUNCTION>::compute_storage(const RealVector& coordinate, SpaceElem& element, std::vector<SpaceElem>& stencil, std::vector<Uint>& points, std::vector<Real>& weights)
{
  // 1) Find the element this coordinate falls in
  const bool element_found = m_element_finder->find_element(coordinate,element);
  if (!element_found)
  {
    // Not interpolated because coordinate not found with element finder
    return false;
  }

  // 2) Find stencil of elements to use
  stencil.clear();
  m_stencil_computer->compute_stencil(element,stencil);

  // 3) Find interpolation
  points.clear();
  weights.clear();
  m_interpolator_function->compute_interpolation_weights(coordinate,stencil,points,weights);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_PointInterpolatorT_hpp
