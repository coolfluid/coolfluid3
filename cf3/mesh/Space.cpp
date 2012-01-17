// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/FindComponents.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/PropertyList.hpp"
#include "common/Link.hpp"

#include "mesh/Space.hpp"
#include "mesh/Elements.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/ShapeFunction.hpp"
#include "mesh/Field.hpp"
#include "mesh/Connectivity.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

common::ComponentBuilder < Space, Component, LibMesh > Space_Builder;

////////////////////////////////////////////////////////////////////////////////

Space::Space ( const std::string& name ) :
  Component ( name )
{
  mark_basic();

  properties()["brief"] = std::string("Spaces are other views of Entities, for instance a higher-order representation");
  properties()["description"] = std::string("");

  options().add_option("shape_function", std::string())
      .description("Shape Function defined in this space")
      .pretty_name("Shape Function")
      .attach_trigger(boost::bind(&Space::configure_shape_function, this))
      .mark_basic();

  m_connectivity = create_static_component<Connectivity>("connectivity");

  m_fields = create_static_component<Link>("fields");
}

////////////////////////////////////////////////////////////////////////////////

Space::~Space()
{
}

const ShapeFunction& Space::shape_function() const
{
  if(is_null(m_shape_function))
    throw SetupError(FromHere(), "Shape function not configured for " + uri().string());
  return *m_shape_function;
}


void Space::set_support(Entities& support)
{
  m_support = Handle<Entities>(support.handle<Component>());
}

Entities& Space::support()
{
  if(is_null(m_support))
    throw SetupError(FromHere(), "Support not set for " + uri().string());

  return *m_support;
}

const Entities& Space::support() const
{
  if(is_null(m_support))
    throw SetupError(FromHere(), "Support not set for " + uri().string());

  return *m_support;
}

ElementType& Space::element_type()
{
  return support().element_type();
}

const ElementType& Space::element_type() const
{
  return support().element_type();
}

Uint Space::nb_states() const
{
  return shape_function().nb_nodes();
}

Uint Space::size() const
{
  return m_connectivity->size();
}

////////////////////////////////////////////////////////////////////////////////

void Space::configure_shape_function()
{
  const std::string sf_name = options().option("shape_function").value<std::string>();
  if (is_not_null(m_shape_function))
  {
    remove_component(m_shape_function->name());
  }
  m_shape_function = create_component<ShapeFunction>(sf_name, sf_name);
  m_shape_function->rename(m_shape_function->derived_type_name());
}

////////////////////////////////////////////////////////////////////////////////

bool Space::is_bound_to_fields() const
{
  return m_fields->is_linked();
}

////////////////////////////////////////////////////////////////////////////////

SpaceFields& Space::fields() const
{
  cf3_assert(is_bound_to_fields());
  return *Handle<SpaceFields>(follow_link(*m_fields));
}

////////////////////////////////////////////////////////////////////////////////

Connectivity::ConstRow Space::indexes_for_element(const Uint elem_idx) const
{
  cf3_assert_desc(connectivity().uri().string()+"["+common::to_str(elem_idx)+"]",elem_idx<connectivity().size());
  return connectivity()[elem_idx];
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix Space::compute_coordinates(const Uint elem_idx) const
{
  const ShapeFunction& space_sf       = shape_function();
  const Entities&     geometry       = support();
  const ElementType&   geometry_etype = element_type();
  const ShapeFunction& geometry_sf    = geometry_etype.shape_function();
  RealMatrix geometry_coordinates = geometry.get_coordinates(elem_idx);
  RealMatrix space_coordinates(space_sf.nb_nodes(),geometry_etype.dimension());
  for (Uint node=0; node<space_sf.nb_nodes(); ++node)
  {
    space_coordinates.row(node) = geometry_sf.value( space_sf.local_coordinates().row(node) ) * geometry_coordinates;
  }
  return space_coordinates;
}

////////////////////////////////////////////////////////////////////////////////

void Space::put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const
{
  Connectivity::ConstRow indexes = indexes_for_element(elem_idx);
  Field& coordinates_field = fields().coordinates();

  cf3_assert(coordinates.rows() == indexes.size());
  cf3_assert(coordinates.cols() == coordinates_field.row_size());

  for (Uint i=0; i<coordinates.rows(); ++i)
  {
    for (Uint j=0; j<coordinates.cols(); ++j)
    {
      coordinates(i,j) = coordinates_field[indexes[i]][j];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void Space::allocate_coordinates(RealMatrix& coordinates) const
{
  coordinates.resize(nb_states(),element_type().dimension());
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix Space::get_coordinates(const Uint elem_idx) const
{
  Connectivity::ConstRow indexes = indexes_for_element(elem_idx);
  Field& coordinates_field = fields().coordinates();
  RealMatrix coordinates(indexes.size(),coordinates_field.row_size());
  for (Uint i=0; i<coordinates.rows(); ++i)
  {
    for (Uint j=0; j<coordinates.cols(); ++j)
    {
      coordinates(i,j) = coordinates_field[indexes[i]][j];
    }
  }
  return coordinates;
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
