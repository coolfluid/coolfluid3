// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/regex.hpp>

#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/OptionArray.hpp"
#include "common/Foreach.hpp"
#include "common/Link.hpp"

#include "common/PE/CommPattern.hpp"

#include "mesh/Field.hpp"
#include "mesh/Region.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/Mesh.hpp"

#include "math/VariablesDescriptor.hpp"

using namespace boost::assign;

using namespace cf3::common;
using namespace cf3::common::PE;

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Field, Component, LibMesh >  Field_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

Field::Field ( const std::string& name  ) :
  common::Table<Real> ( name ),
  m_basis(SpaceFields::Basis::INVALID)
{
  mark_basic();

//  options().add_option(1,name))
//      .description("Names of the variables")
//      .pretty_name("Variable Names")
//      .attach_trigger ( boost::bind ( &Field::config_var_names, this ) )
//      .mark_basic();
//  config_var_names();

//  options().add_option(1,"scalar"))
//      .description("Types of the variables")
//      .attach_trigger ( boost::bind ( &Field::config_var_types,   this ) )
//      .mark_basic()
//      ->restricted_list() = boost::assign::list_of
//        (std::string("scalar"))
//        (std::string("vector2D"))
//        (std::string("vector3D"))
//        (std::string("tensor2D"))
//        (std::string("tensor3D"));
//  config_var_types();


}


Field::~Field() {}


void Field::config_var_types()
{
}


void Field::config_var_names()
{
}

////////////////////////////////////////////////////////////////////////////////

void Field::set_topology(Region& region)
{
  m_topology = Handle<Region>(region.handle<Component>());
}

////////////////////////////////////////////////////////////////////////////////

Uint Field::nb_vars() const
{
  return descriptor().nb_vars();
}

////////////////////////////////////////////////////////////////////////////////

bool Field::has_variable(const std::string& vname) const
{
  return descriptor().has_variable(vname);
}

////////////////////////////////////////////////////////////////////////////////

std::string Field::var_name(Uint var_nb) const
{
  return descriptor().user_variable_name(var_nb);
}

////////////////////////////////////////////////////////////////////////////////

Uint Field::var_number ( const std::string& vname ) const
{
  return descriptor().var_number(vname);
}

//////////////////////////////////////////////////////////////////////////////

Uint Field::var_index ( const std::string& vname ) const
{
  return descriptor().offset(vname);
}

////////////////////////////////////////////////////////////////////////////////

Uint Field::var_index ( const Uint var_nb ) const
{
  return descriptor().offset(var_nb);
}

//////////////////////////////////////////////////////////////////////////////

Field::VarType Field::var_length(const Uint var_nb) const
{
  return (Field::VarType)descriptor().var_length(var_nb);
}

//////////////////////////////////////////////////////////////////////////////

Field::VarType Field::var_length ( const std::string& vname ) const
{
  return (Field::VarType)descriptor().var_length(vname);
}

////////////////////////////////////////////////////////////////////////////////

Region& Field::topology() const
{
  cf3_assert(is_null(m_topology) == false);
  return *m_topology;
}

////////////////////////////////////////////////////////////////////////////////

void Field::set_field_group(SpaceFields& field_group)
{
  m_field_group = Handle<SpaceFields>(field_group.handle<Component>());
}

////////////////////////////////////////////////////////////////////////////////

SpaceFields& Field::field_group() const
{
  cf3_assert(is_null(m_field_group) == false);
  return *m_field_group;
}

////////////////////////////////////////////////////////////////////////////////

void Field::resize(const Uint size)
{
  set_row_size(descriptor().size());
  common::Table<Real>::resize(size);
}

////////////////////////////////////////////////////////////////////////////////

//common::Table<Uint>::ConstRow Field::indexes_for_element(const Entities& elements, const Uint idx) const
//{
//  return field_group().indexes_for_element(elements,idx);
//}


//common::Table<Uint>::ConstRow Field::indexes_for_element(const Uint unified_idx) const
//{
//  return field_group().indexes_for_element(unified_idx);
//}

std::vector< Handle<Entities> > Field::entities_range()
{
  return field_group().entities_range();
}

std::vector< Handle<Elements> > Field::elements_range()
{
  return field_group().elements_range();
}

CommPattern& Field::parallelize_with(CommPattern& comm_pattern)
{
  m_comm_pattern = Handle<CommPattern>(comm_pattern.handle<Component>());
  comm_pattern.insert(name(), array(), true);
  return comm_pattern;
}


CommPattern& Field::parallelize()
{
  CommPattern& comm_pattern = field_group().comm_pattern();

  // Do nothing if parallel already
  if(is_not_null(comm_pattern.get_child(name())))
    return comm_pattern;

  return parallelize_with( comm_pattern );
}


void Field::synchronize()
{
  if ( is_not_null(m_comm_pattern) )
    m_comm_pattern->synchronize( name() );
}

////////////////////////////////////////////////////////////////////////////////////////////

void Field::set_descriptor(math::VariablesDescriptor& descriptor)
{
  if (Handle< math::VariablesDescriptor > old_descriptor = find_component_ptr<math::VariablesDescriptor>(*this))
    remove_component(*old_descriptor);
  m_descriptor = Handle<math::VariablesDescriptor>(descriptor.handle<Component>());
}

////////////////////////////////////////////////////////////////////////////////////////////

void Field::create_descriptor(const std::string& description, const Uint dimension)
{
  if (Handle< math::VariablesDescriptor > old_descriptor = find_component_ptr<math::VariablesDescriptor>(*this))
    remove_component(*old_descriptor);
  m_descriptor = create_component<math::VariablesDescriptor>("description");
  descriptor().set_variables(description,dimension);
}

////////////////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
