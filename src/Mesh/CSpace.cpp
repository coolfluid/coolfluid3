// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/FindComponents.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/CLink.hpp"

#include "Mesh/CSpace.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CEntities.hpp"
#include "Mesh/CConnectivity.hpp"
#include "Mesh/FieldGroup.hpp"
#include "Mesh/ShapeFunction.hpp"
#include "Mesh/Field.hpp"

namespace cf3 {
namespace Mesh {

using namespace common;

common::ComponentBuilder < CSpace, Component, LibMesh > CSpace_Builder;

////////////////////////////////////////////////////////////////////////////////

CSpace::CSpace ( const std::string& name ) :
  Component ( name ),
  m_is_proxy(false),
  m_elem_start_idx(0)
{
  mark_basic();

  m_properties["brief"] = std::string("Spaces are other views of CEntities, for instance a higher-order representation");
  m_properties["description"] = std::string("");

  m_options.add_option(OptionT<std::string>::create("shape_function", std::string("")))
      ->description("Shape Function defined in this space")
      ->pretty_name("Shape Function")
      ->attach_trigger(boost::bind(&CSpace::configure_shape_function, this))
      ->mark_basic();

  m_connectivity = create_static_component_ptr<CConnectivity>("connectivity");

  m_bound_fields = create_static_component_ptr<CLink>("bound_fields");
}

////////////////////////////////////////////////////////////////////////////////

CSpace::~CSpace()
{
}

const ShapeFunction& CSpace::shape_function() const
{
  if(is_null(m_shape_function))
    throw SetupError(FromHere(), "Shape function not configured for " + uri().string());
  return *m_shape_function;
}


void CSpace::set_support(CEntities& support)
{
  m_support = support.as_ptr<CEntities>();
}

CEntities& CSpace::support()
{
  if(m_support.expired())
    throw SetupError(FromHere(), "Support not set for " + uri().string());

  return *m_support.lock();
}

const CEntities& CSpace::support() const
{
  if(m_support.expired())
    throw SetupError(FromHere(), "Support not set for " + uri().string());

  return *m_support.lock();
}


////////////////////////////////////////////////////////////////////////////////

void CSpace::configure_shape_function()
{
  const std::string sf_name = option("shape_function").value<std::string>();
  if (is_not_null(m_shape_function))
  {
    remove_component(m_shape_function->name());
  }
  m_shape_function = build_component_abstract_type<ShapeFunction>( sf_name, sf_name );
  m_shape_function->rename(m_shape_function->derived_type_name());
  add_component( m_shape_function );
}

////////////////////////////////////////////////////////////////////////////////

bool CSpace::is_bound_to_fields() const
{
  return m_bound_fields->is_linked();
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup& CSpace::bound_fields() const
{
  cf_assert(is_bound_to_fields());
  return m_bound_fields->follow()->as_type<FieldGroup>();
}

////////////////////////////////////////////////////////////////////////////////

CConnectivity::ConstRow CSpace::indexes_for_element(const Uint elem_idx) const
{
  if (m_is_proxy)
  {
    const Uint start_idx = m_elem_start_idx+elem_idx*m_connectivity_proxy.shape()[1];
    for (Uint i=0; i<m_connectivity_proxy.shape()[1]; ++i)
      m_connectivity_proxy[0][i] = start_idx+i;
    return m_connectivity_proxy[0];
  }
  else
  {
    return connectivity()[elem_idx];
  }
}

////////////////////////////////////////////////////////////////////////////////

void CSpace::make_proxy(const Uint elem_start_idx)
{
  m_is_proxy = true;
  m_elem_start_idx = elem_start_idx;
  m_connectivity_proxy.resize(boost::extents[1][nb_states()]);
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix CSpace::compute_coordinates(const Uint elem_idx) const
{
  const ShapeFunction& space_sf       = shape_function();
  const CEntities&     geometry       = support();
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

void CSpace::put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const
{
  if (bound_fields().has_coordinates())
  {
    CConnectivity::ConstRow indexes = indexes_for_element(elem_idx);
    Field& coordinates_field = bound_fields().coordinates();

    cf_assert(coordinates.rows() == indexes.size());
    cf_assert(coordinates.cols() == coordinates_field.row_size());

    for (Uint i=0; i<coordinates.rows(); ++i)
    {
      for (Uint j=0; j<coordinates.cols(); ++j)
      {
        coordinates(i,j) = coordinates_field[i][j];
      }
    }
  }
  else
  {
    const ShapeFunction& space_sf       = shape_function();
    const CEntities&     geometry       = support();
    const ElementType&   geometry_etype = element_type();
    const ShapeFunction& geometry_sf    = geometry_etype.shape_function();
    RealMatrix geometry_coordinates = geometry.get_coordinates(elem_idx);

    cf_assert(coordinates.rows() == space_sf.nb_nodes());
    cf_assert(coordinates.cols() == geometry_etype.dimension());
    for (Uint node=0; node<space_sf.nb_nodes(); ++node)
    {
      coordinates.row(node) = geometry_sf.value( space_sf.local_coordinates().row(node) ) * geometry_coordinates;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void CSpace::allocate_coordinates(RealMatrix& coordinates) const
{
  coordinates.resize(nb_states(),element_type().dimension());
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix CSpace::get_coordinates(const Uint elem_idx) const
{
  if (bound_fields().has_coordinates())
  {

    CConnectivity::ConstRow indexes = indexes_for_element(elem_idx);
    Field& coordinates_field = bound_fields().coordinates();
    RealMatrix coordinates(indexes.size(),coordinates_field.row_size());
    for (Uint i=0; i<coordinates.rows(); ++i)
    {
      for (Uint j=0; j<coordinates.cols(); ++j)
      {
        coordinates(i,j) = coordinates_field[i][j];
      }
    }
    return coordinates;
  }
  else
  {
    return compute_coordinates(elem_idx);
  }
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // cf3
