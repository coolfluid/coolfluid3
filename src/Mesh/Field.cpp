// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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

#include "Common/MPI/CommPattern.hpp"

#include "Mesh/Field.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CMesh.hpp"

using namespace boost::assign;

using namespace CF::Common;
using namespace CF::Common::Comm;

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

  m_options.add_option<OptionArrayT<std::string> >("var_names", std::vector<std::string>(1,name))
      ->description("Names of the variables")
      ->pretty_name("Variable Names")
      ->attach_trigger ( boost::bind ( &Field::config_var_names, this ) )
      ->mark_basic();
  config_var_names();

  m_options.add_option<OptionArrayT<std::string> >("var_types", std::vector<std::string>(1,"scalar"))
      ->description("Types of the variables")
      ->attach_trigger ( boost::bind ( &Field::config_var_types,   this ) )
      ->mark_basic()
      ->restricted_list() = boost::assign::list_of
        (std::string("scalar"))
        (std::string("vector2D"))
        (std::string("vector3D"))
        (std::string("tensor2D"))
        (std::string("tensor3D"));
  config_var_types();

}


Field::~Field() {}


void Field::config_var_types()
{
  std::vector<std::string> var_types; option("var_types").put_value(var_types);

  boost::regex e_scalar  ("(s(cal(ar)?)?)|1"     ,boost::regex::perl|boost::regex::icase);
  boost::regex e_vector2d("(v(ec(tor)?)?.?2d?)|2",boost::regex::perl|boost::regex::icase);
  boost::regex e_vector3d("(v(ec(tor)?)?.?3d?)|3",boost::regex::perl|boost::regex::icase);
  boost::regex e_tensor2d("(t(ens(or)?)?.?2d?)|4",boost::regex::perl|boost::regex::icase);
  boost::regex e_tensor3d("(t(ens(or)?)?.?3d?)|9",boost::regex::perl|boost::regex::icase);

  m_var_types.resize(var_types.size());
  Uint iVar = 0;
  boost_foreach (std::string& var_type, var_types)
  {
    if (boost::regex_match(var_type,e_scalar))
    {
      var_type="scalar";
      m_var_types[iVar++]=SCALAR;
    }
    else if (boost::regex_match(var_type,e_vector2d))
    {
      var_type="vector_2D";
      m_var_types[iVar++]=VECTOR_2D;
    }
    else if (boost::regex_match(var_type,e_vector3d))
    {
      var_type="vector_3D";
      m_var_types[iVar++]=VECTOR_3D;
    }
    else if (boost::regex_match(var_type,e_tensor2d))
    {
      var_type="tensor_2D";
      m_var_types[iVar++]=TENSOR_2D;
    }
    else if (boost::regex_match(var_type,e_tensor3d))
    {
      var_type="tensor_3D";
      m_var_types[iVar++]=TENSOR_3D;
    }
  }
  // give property a similar look, not all possible regex combinations
  //option("VarTypes").change_value(var_types); // this triggers infinite recursion
}


void Field::config_var_names()
{
  option("var_names").put_value(m_var_names);
}


std::string Field::var_name(Uint i) const
{
  cf_assert(i<m_var_types.size());
  return m_var_names.size() ? m_var_names[i] : "var";

  //  std::vector<std::string> names;
  //  switch (m_var_types[i])
  //  {
  //    case SCALAR:
  //      names += name;
  //      break;
  //    case VECTOR_2D:
  //      names += name+"x";
  //      names += name+"y";
  //      break;
  //    case VECTOR_3D:
  //      names += name+"x";
  //      names += name+"y";
  //      names += name+"z";
  //      break;
  //    case TENSOR_2D:
  //      names += name+"xx";
  //      names += name+"xy";
  //      names += name+"yx";
  //      names += name+"yy";
  //      break;
  //    case TENSOR_3D:
  //      names += name+"xx";
  //      names += name+"xy";
  //      names += name+"xz";
  //      names += name+"yx";
  //      names += name+"yy";
  //      names += name+"yz";
  //      names += name+"zx";
  //      names += name+"zy";
  //      names += name+"zz";
  //      break;
  //    default:
  //      break;
  //  }
  //  return names;
}


void Field::set_topology(CRegion& region)
{
  m_topology = region.as_ptr<CRegion>();
}


////////////////////////////////////////////////////////////////////////////////

bool Field::has_variable(const std::string& vname) const
{
  return std::find(m_var_names.begin(), m_var_names.end(), vname) != m_var_names.end();
}

////////////////////////////////////////////////////////////////////////////////

Uint Field::var_number ( const std::string& vname ) const
{
  const std::vector<std::string>::const_iterator var_loc_it = std::find(m_var_names.begin(), m_var_names.end(), vname);
  if(var_loc_it == m_var_names.end())
    throw Common::ValueNotFound(FromHere(), "Variable " + vname + " was not found in field " + name());
  return var_loc_it - m_var_names.begin();
}
//////////////////////////////////////////////////////////////////////////////

Uint Field::var_index ( const std::string& vname ) const
{
  const Uint var_nb = var_number(vname);
  return var_index(var_nb);
}

////////////////////////////////////////////////////////////////////////////////

Uint Field::var_index ( const Uint var_nb ) const
{
  Uint var_start = 0;
  for(Uint i = 0; i != var_nb; ++i)
    var_start += m_var_types[i];
  return var_start;
}

//////////////////////////////////////////////////////////////////////////////

Field::VarType Field::var_type ( const std::string& vname ) const
{
  return var_type(var_number(vname));
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
  Uint row_size(0);
  boost_foreach(const VarType var_size, m_var_types)
    row_size += Uint(var_size);

  set_row_size(row_size);
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
  cf_assert(glb_idx().size() == size());
  cf_assert(rank().size()    == size());
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

} // Mesh
} // CF
