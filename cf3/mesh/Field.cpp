// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/date_time/gregorian/gregorian.hpp>

#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/OptionArray.hpp"
#include "common/Foreach.hpp"
#include "common/Link.hpp"

#include "common/PE/CommPattern.hpp"

#include "mesh/Field.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Mesh.hpp"

#include "math/VariablesDescriptor.hpp"

using namespace cf3::common;
using namespace cf3::common::PE;

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Field, Component, LibMesh >  Field_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

Field::Field ( const std::string& name  ) :
  common::Table<Real> ( name ), m_var_type(ARRAY)
{
  mark_basic();
  properties()["date"] = boost::gregorian::to_iso_extended_string(boost::gregorian::day_clock::local_day());
  properties()["time"] = 0.;
  properties()["step"] = 0u;
}

////////////////////////////////////////////////////////////////////////////////

Field::~Field() {}

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

Uint Field::var_offset ( const std::string& vname ) const
{
  return descriptor().offset(vname);
}

////////////////////////////////////////////////////////////////////////////////

Uint Field::var_offset ( const Uint var_nb ) const
{
  return descriptor().offset(var_nb);
}

//////////////////////////////////////////////////////////////////////////////

VarType Field::var_length(const Uint var_nb) const
{
  return (VarType)descriptor().var_length(var_nb);
}

//////////////////////////////////////////////////////////////////////////////

VarType Field::var_length ( const std::string& vname ) const
{
  return (VarType)descriptor().var_length(vname);
}

////////////////////////////////////////////////////////////////////////////////

void Field::set_dict(Dictionary& dict)
{
  m_dict = dict.handle<Dictionary>();
}

////////////////////////////////////////////////////////////////////////////////

Dictionary& Field::dict() const
{
  cf3_assert(is_null(m_dict) == false);
  return *m_dict;
}

////////////////////////////////////////////////////////////////////////////////

const std::vector< Handle<Entities> >& Field::entities_range() const
{
  return dict().entities_range();
}

//////////////////////////////////////////////////////////////////////////////////

const std::vector< Handle<Space> >& Field::spaces() const
{
  return dict().spaces();
}

////////////////////////////////////////////////////////////////////////////////

CommPattern& Field::parallelize_with(CommPattern& comm_pattern)
{
  m_comm_pattern = Handle<CommPattern>(comm_pattern.handle<Component>());
  comm_pattern.insert(name(), array(), true);
  return comm_pattern;
}

////////////////////////////////////////////////////////////////////////////////

CommPattern& Field::parallelize()
{
  CommPattern& comm_pattern = dict().comm_pattern();

  // Do nothing if parallel already
  if(is_not_null(comm_pattern.get_child(name())))
    return comm_pattern;

  return parallelize_with( comm_pattern );
}

////////////////////////////////////////////////////////////////////////////////

void Field::synchronize()
{
  if ( is_not_null(m_comm_pattern) )
  {
    CFdebug << "Synchronizing field " << uri().path() << CFendl;
    m_comm_pattern->synchronize( name() );
  }
  else
  {
    CFdebug << "Not synchronizing field " << uri().path() << " due to null comm pattern" << CFendl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void Field::set_descriptor(math::VariablesDescriptor& descriptor)
{
  if (Handle< math::VariablesDescriptor > old_descriptor = find_component_ptr<math::VariablesDescriptor>(*this))
    remove_component(*old_descriptor);
  m_descriptor = descriptor.handle<math::VariablesDescriptor>();
}

////////////////////////////////////////////////////////////////////////////////////////////

void Field::create_descriptor(const std::string& description, const Uint dimension)
{
  if (Handle< math::VariablesDescriptor > old_descriptor = find_component_ptr<math::VariablesDescriptor>(*this))
    remove_component(*old_descriptor);
  m_descriptor = create_component<math::VariablesDescriptor>("description");
  descriptor().set_variables(description,dimension);
}

////////////////////////////////////////////////////////////////////////////////

Field::View Field::view(const Uint start, const Uint size)
{
  return Table<Real>::array()[ boost::indices[range(start,start+size)][range()] ];
}

////////////////////////////////////////////////////////////////////////////////////////////

Field::View Field::view(common::Table<Uint>::ConstRow& indices)
{
  return Table<Real>::array()[ boost::indices[range(indices[0],indices[0]+indices.size())][range()] ];
}

////////////////////////////////////////////////////////////////////////////////////////////

common::List<Uint>& Field::glb_idx() const
{
  return dict().glb_idx();
}

////////////////////////////////////////////////////////////////////////////////////////////

common::List<Uint>& Field::rank() const
{
  return dict().rank();
}

////////////////////////////////////////////////////////////////////////////////////////////

bool Field::is_ghost(const Uint idx) const
{
  return dict().is_ghost(idx);
}

////////////////////////////////////////////////////////////////////////////////////////////

bool Field::continuous() const
{
  return dict().continuous();
}
////////////////////////////////////////////////////////////////////////////////////////////

bool Field::discontinuous() const
{
  return dict().discontinuous();
}

////////////////////////////////////////////////////////////////////////////////////////////

const Handle<Space const>& Field::space(const Handle<Entities const>& entities) const
{
  return dict().space(entities);
}

////////////////////////////////////////////////////////////////////////////////////////////

const Space& Field::space(const Entities& entities) const
{
  return dict().space(entities);
}

////////////////////////////////////////////////////////////////////////////////////////////

Field& Field::coordinates() const
{
  return dict().coordinates();
}

////////////////////////////////////////////////////////////////////////////////////////////

Field::Ref Field::ref()
{
  return Ref( &array()[0][0], size(), row_size(), Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic>(row_size(),row_size()-1));
}

////////////////////////////////////////////////////////////////////////////////

Field::Ref  Field::col(const Uint c)
{
  return Ref( &array()[0][c], size(), 1, Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic>(row_size(),0) );
}

////////////////////////////////////////////////////////////////////////////////

Field::RowArrayRef  Field::row(const Uint r)
{
  return RowArrayRef( &array()[r][0], 1, row_size() );
}

////////////////////////////////////////////////////////////////////////////////

Real& Field::scalar(const Uint r)
{
  return array()[r][0];
}

////////////////////////////////////////////////////////////////////////////////

Field::RowVectorRef Field::vector(const Uint r)
{
  return RowVectorRef( &array()[r][0], 1, row_size() );
}

////////////////////////////////////////////////////////////////////////////////

Field::RowTensorRef Field::tensor(const Uint r)
{
  return RowTensorRef( &array()[r][0],  sqrt(row_size()), sqrt(row_size()));
}

////////////////////////////////////////////////////////////////////////////////

void Field::set_var_type(const VarType var_type)
{
  m_var_type = var_type;
}

////////////////////////////////////////////////////////////////////////////////

VarType Field::var_type() const
{
  return m_var_type;
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
