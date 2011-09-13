// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/regex.hpp>

#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionArray.hpp"
#include "Common/Foreach.hpp"
#include "Common/CLink.hpp"

#include "Common/PE/CommPattern.hpp"

#include "Mesh/Field.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CMesh.hpp"

#include "Math/VariablesDescriptor.hpp"

using namespace boost::assign;

using namespace CF::Common;
using namespace CF::Common::PE;

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Field, Component, LibMesh >  Field_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

Field::Field ( const std::string& name  ) :
  CTable<Real> ( name ),
  m_basis(FieldGroup::Basis::INVALID)
{
  mark_basic();

//  m_options.add_option<OptionArrayT<std::string> >("var_names", std::vector<std::string>(1,name))
//      ->description("Names of the variables")
//      ->pretty_name("Variable Names")
//      ->attach_trigger ( boost::bind ( &Field::config_var_names, this ) )
//      ->mark_basic();
//  config_var_names();

//  m_options.add_option<OptionArrayT<std::string> >("var_types", std::vector<std::string>(1,"scalar"))
//      ->description("Types of the variables")
//      ->attach_trigger ( boost::bind ( &Field::config_var_types,   this ) )
//      ->mark_basic()
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

void Field::set_topology(CRegion& region)
{
  m_topology = region.as_ptr<CRegion>();
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

CRegion& Field::topology() const
{
  cf_assert(m_topology.expired() == false);
  return *m_topology.lock();
}

////////////////////////////////////////////////////////////////////////////////

void Field::set_field_group(FieldGroup& field_group)
{
  m_field_group = field_group.as_ptr<FieldGroup>();
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup& Field::field_group() const
{
  cf_assert(m_field_group.expired() == false);
  return *m_field_group.lock();
}

////////////////////////////////////////////////////////////////////////////////

void Field::resize(const Uint size)
{
  set_row_size(descriptor().size());
  CTable<Real>::resize(size);
}

////////////////////////////////////////////////////////////////////////////////

CTable<Uint>::ConstRow Field::indexes_for_element(const CEntities& elements, const Uint idx) const
{
  return field_group().indexes_for_element(elements,idx);
}


CTable<Uint>::ConstRow Field::indexes_for_element(const Uint unified_idx) const
{
  return field_group().indexes_for_element(unified_idx);
}


CommPattern& Field::parallelize_with(CommPattern& comm_pattern)
{
  cf_assert_desc("Only point-based fields supported now", m_basis == FieldGroup::Basis::POINT_BASED);
  m_comm_pattern = comm_pattern.as_ptr<CommPattern>();
  comm_pattern.insert(name(), array(), true);
  return comm_pattern;
}


CommPattern& Field::parallelize()
{
  if ( !m_comm_pattern.expired() ) // return if already parallel
    return *m_comm_pattern.lock();

  // Extract gid from the nodes.glb_idx()  for only the nodes in the region the fields will use.
  std::vector<Uint> gid;
  std::vector<Uint> ranks;
  gid.reserve( size() );
  ranks.reserve( size() );

  CMesh& mesh = find_parent_component<CMesh>(*this);
  cf_assert_desc("["+to_str(glb_idx().size())+"!="+to_str(size())+"]",glb_idx().size() == size());
  cf_assert_desc("["+to_str(rank().size())+"!="+to_str(size())+"]",rank().size() == size());
  for (Uint node=0; node<size(); ++node)
  {
    gid.push_back( glb_idx()[node] );
    ranks.push_back( rank()[node] );
  }

  // create the comm pattern and setup the pattern

  CommPattern& comm_pattern = mesh.create_component<CommPattern>("comm_pattern_node_based");

  comm_pattern.insert("gid",gid,1,false);
  comm_pattern.setup(comm_pattern.get_child("gid").as_ptr<CommWrapper>(),ranks);

  return parallelize_with( comm_pattern );
}


void Field::synchronize()
{
  if ( !m_comm_pattern.expired() )
    m_comm_pattern.lock()->synchronize( name() );
}

////////////////////////////////////////////////////////////////////////////////////////////

void Field::set_descriptor(Math::VariablesDescriptor& descriptor)
{
  if (Math::VariablesDescriptor::Ptr old_descriptor = find_component_ptr<Math::VariablesDescriptor>(*this))
    remove_component(*old_descriptor);
  m_descriptor = descriptor.as_ptr<Math::VariablesDescriptor>();
}

////////////////////////////////////////////////////////////////////////////////////////////

void Field::create_descriptor(const std::string& description, const Uint dimension)
{
  if (Math::VariablesDescriptor::Ptr old_descriptor = find_component_ptr<Math::VariablesDescriptor>(*this))
    remove_component(*old_descriptor);
  m_descriptor = create_component_ptr<Math::VariablesDescriptor>("description");
  descriptor().set_variables(description,dimension);
}

////////////////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
