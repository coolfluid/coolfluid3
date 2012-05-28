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

#include "mesh/Space.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/ShapeFunction.hpp"
#include "mesh/Field.hpp"

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

  std::stringstream msg;
  msg << "A Space component uniquely relates to 1 Entities component. \n"
      << "The concept of \"space\" is here introduced as an invisible mesh parallel\n"
      << "to the original mesh. It has exactly the same elements as the original mesh,\n"
      << "but every element is defined by a different shape function.\n"
      << "A default space that is always created is the \"geometry\" space, defined by\n"
      << "the mesh (from mesh-readers / mesh-generators).\n"
      << "Space is a concept that allows to create fields in the same mesh or parts of the\n"
      << "mesh, with different shape functions than prescribed by the mesh.\n"
      << "This is useful for e.g. high-order discretization methods, without having to\n"
      << "generate a high-order mesh.\n"
      << "A class Dictionary is responsible for managing multiple Space components.\n"
      << "Fields are created in the dictionary. More than one field can be created in he same\n"
      << "dictionary, ensuring they have the same space and other common definitions.\n"
      << "A connectivity table which is held inside this Space component refers to entries\n"
      << "in the dictionary.\n"
      << "\n"
      << "Example:\n"
      << "The coordinates of mesh element vertices is e.g. a field in the dictionary \"geometry\",\n"
      << "and the connectivity table of the space \"geometry\" refers to the\n"
      << "vertices connected to the mesh-elements.\n"
      << "\n"
      << "Notes:\n"
      << "- Newly created spaces always have a coordinates field in their dictionary.\n";

  properties()["description"] = msg.str();

  options().add("shape_function", std::string())
      .description("Shape Function defined in this space")
      .pretty_name("Shape Function")
      .attach_trigger(boost::bind(&Space::configure_shape_function, this))
      .mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

Space& Space::initialize(Entities& support, Dictionary& dict)
{
  m_support = support.handle<Entities>();
  m_dict = dict.handle<Dictionary>();
  m_connectivity = create_static_component<Connectivity>("connectivity");
  dict.add_space(this->handle<Space>());
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

Space::~Space()
{
}

////////////////////////////////////////////////////////////////////////////////

Uint Space::size() const
{
  return m_connectivity->size();
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& Space::shape_function() const
{
  if(is_null(m_shape_function))
    throw SetupError(FromHere(), "Shape function not configured for " + uri().string());
  return *m_shape_function;
}

////////////////////////////////////////////////////////////////////////////////

void Space::configure_shape_function()
{
  if (is_null(m_dict))
    throw SetupError(FromHere(), "Space "+uri().string()+" must be created using Entities::create_space()");

  const std::string sf_name = options().value<std::string>("shape_function");
  if (is_not_null(m_shape_function))
  {
    if (m_shape_function->derived_type_name() != sf_name)
    {
      if ( m_dict->size() !=0 )
        throw NotImplemented(FromHere(), "Changing the shape-function will affect the dictionary, which is not yet supported");
    }
  }
  else
  {
    m_shape_function = create_component<ShapeFunction>(sf_name, sf_name);
    m_shape_function->rename(m_shape_function->derived_type_name());
    m_connectivity->set_row_size(m_shape_function->nb_nodes());
    m_connectivity->resize(m_support->size());
  }
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix Space::compute_coordinates(const Uint elem_idx) const
{
  const ShapeFunction& space_sf       = shape_function();
  const Entities&     geometry       = support();
  const ElementType&   geometry_etype = geometry.element_type();
  const ShapeFunction& geometry_sf    = geometry_etype.shape_function();
  RealMatrix geometry_coordinates = geometry.geometry_space().get_coordinates(elem_idx);
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
  Connectivity::ConstRow indexes = connectivity()[elem_idx];
  Field& coordinates_field = dict().coordinates();

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
  coordinates.resize(shape_function().nb_nodes(),support().element_type().dimension());
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix Space::get_coordinates(const Uint elem_idx) const
{
  Connectivity::ConstRow indexes = connectivity()[elem_idx];
  Field& coordinates_field = dict().coordinates();
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
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

RegistTypeInfo<SpaceElem,LibMesh> regist_SpaceElem;

////////////////////////////////////////////////////////////////////////////////

SpaceElem::SpaceElem() :
  comp(NULL),
  idx(0)
{
}

////////////////////////////////////////////////////////////////////////////////

SpaceElem::SpaceElem(const SpaceElem& other) :
  comp(other.comp),
  idx(other.idx)
{
}

////////////////////////////////////////////////////////////////////////////////

SpaceElem::SpaceElem(Space& space, const Uint index) :
  comp( &space ),
  idx(index)
{
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& SpaceElem::shape_function() const
{
  return comp->shape_function();
}

////////////////////////////////////////////////////////////////////////////////

Uint SpaceElem::glb_idx() const
{
  return comp->support().glb_idx()[idx];
}

////////////////////////////////////////////////////////////////////////////////

Uint SpaceElem::rank() const
{
  return comp->support().rank()[idx];
}

////////////////////////////////////////////////////////////////////////////////

bool SpaceElem::is_ghost() const
{
  return comp->support().is_ghost(idx);
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix SpaceElem::get_coordinates() const
{
  return comp->get_coordinates(idx);
}

////////////////////////////////////////////////////////////////////////////////

void SpaceElem::put_coordinates(RealMatrix& coordinates) const
{
  comp->put_coordinates(coordinates,idx);
}

////////////////////////////////////////////////////////////////////////////////

void SpaceElem::allocate_coordinates(RealMatrix& coordinates) const
{
  comp->allocate_coordinates(coordinates);
}

////////////////////////////////////////////////////////////////////////////////

Connectivity::ConstRow SpaceElem::nodes() const
{
  return comp->connectivity()[idx];
}

////////////////////////////////////////////////////////////////////////////////

bool SpaceElem::operator==(const SpaceElem& other) const
{
  return comp==other.comp && idx==other.idx;
}

////////////////////////////////////////////////////////////////////////////////

bool SpaceElem::operator!=(const SpaceElem& other) const
{
  return !(*this==other);
}

////////////////////////////////////////////////////////////////////////////////

bool SpaceElem::operator<(const SpaceElem& other) const
{
  return glb_idx()<other.glb_idx();
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const SpaceElem& elem)
{
  cf3_assert(is_not_null(elem.comp));
  os << elem.comp->uri().string()<<"["<<elem.idx<<"]";
  return os;
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
