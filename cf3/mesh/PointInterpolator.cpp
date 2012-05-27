// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "math/MatrixTypesConversion.hpp"

#include "common/FindComponents.hpp"
#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"

#include "mesh/PointInterpolator.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/ElementFinder.hpp"
#include "mesh/StencilComputer.hpp"
#include "mesh/InterpolationFunction.hpp"



#include "common/OptionList.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

APointInterpolator::APointInterpolator ( const std::string& name  ) :
  Component ( name )
{
  options().add("dict", m_dict)
      .description("Dictionary to interpolate from")
      .pretty_name("Source Dictionary")
      .mark_basic()
      .attach_trigger( boost::bind( &APointInterpolator::configure_dict, this ) );
}

////////////////////////////////////////////////////////////////////////////////

void APointInterpolator::configure_dict()
{
  Handle<Dictionary> old_dict = m_dict;
  m_dict = options().value< Handle<Dictionary> >("dict");
  if (m_dict != old_dict)
  {
    boost_foreach ( Component& child, *this )
      child.configure_option_recursively("dict",m_dict);
  }
}

////////////////////////////////////////////////////////////////////////////////

cf3::common::ComponentBuilder<PointInterpolator,APointInterpolator,LibMesh> PointInterpolator_builder;

////////////////////////////////////////////////////////////////////////////////

PointInterpolator::PointInterpolator ( const std::string& name  ) :
  APointInterpolator ( name )
{
  options().add("element_finder", std::string("cf3.mesh.ElementFinderOcttree"))
      .description("Builder name of the element finder")
      .pretty_name("Element Finder")
      .attach_trigger( boost::bind( &PointInterpolator::configure_element_finder, this ) )
      .mark_basic();

  options().add("stencil_computer", std::string("cf3.mesh.StencilComputerOneCell"))
      .description("Builder name of the stencil computer")
      .pretty_name("Stencil Computer")
      .attach_trigger( boost::bind( &PointInterpolator::configure_stencil_computer, this ) )
      .mark_basic();

//  options().add("function", std::string("cf3.mesh.ShapeFunctionInterpolation"))
  options().add("function", std::string("cf3.mesh.PseudoLaplacianLinearInterpolation"))
      .description("Builder name of the interpolator function")
      .pretty_name("Interpolator Function")
      .attach_trigger( boost::bind( &PointInterpolator::configure_interpolator_function, this ) )
      .mark_basic();

  configure_element_finder();
  configure_stencil_computer();
  configure_interpolator_function();
}

////////////////////////////////////////////////////////////////////////////////

void PointInterpolator::configure_element_finder()
{
  if (is_not_null(m_element_finder))
    remove_component(m_element_finder->name());
  m_element_finder = Handle<ElementFinder>(create_component("element_finder",options().value<std::string>("element_finder")));

  if(m_dict)
    m_element_finder->options().set("dict",m_dict);
}

////////////////////////////////////////////////////////////////////////////////

void PointInterpolator::configure_stencil_computer()
{
  if (is_not_null(m_stencil_computer))
    remove_component(m_stencil_computer->name());
  m_stencil_computer = Handle<StencilComputer>(create_component("stencil_computer",options().value<std::string>("stencil_computer")));

  if(m_dict)
    m_stencil_computer->options().set("dict",m_dict);
}

////////////////////////////////////////////////////////////////////////////////

void PointInterpolator::configure_interpolator_function()
{
  if (is_not_null(m_interpolator_function))
    remove_component(m_interpolator_function->name());
  m_interpolator_function = Handle<InterpolationFunction>(create_component("function",options().value<std::string>("function")));

  if(m_dict)
    m_interpolator_function->options().set("dict",m_dict);
}

////////////////////////////////////////////////////////////////////////////////

bool PointInterpolator::compute_storage(const RealVector& coordinate, SpaceElem& element, std::vector<SpaceElem>& stencil, std::vector<Uint>& points, std::vector<Real>& weights)
{
  // 1) Find the element this coordinate falls in
  cf3_assert(m_element_finder);
  const bool element_found = m_element_finder->find_element(coordinate,element);
  if (!element_found)
  {
    // Not interpolated because coordinate not found with element finder
    return false;
  }

  // 2) Find stencil of elements to use
  cf3_assert(m_stencil_computer);
  stencil.clear();
  m_stencil_computer->compute_stencil(element,stencil);

  // 3) Find interpolation
  cf3_assert(m_interpolator_function);
  points.clear();
  weights.clear();
  m_interpolator_function->compute_interpolation_weights(coordinate,stencil,points,weights);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
