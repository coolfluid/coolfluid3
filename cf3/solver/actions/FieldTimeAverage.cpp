// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/List.hpp"
#include "common/PropertyList.hpp"
#include "common/PE/Comm.hpp"
#include "common/Signal.hpp"
#include "common/XML/SignalOptions.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/Dictionary.hpp"

#include "solver/actions/FieldTimeAverage.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < FieldTimeAverage, common::Action, LibActions > FieldTimeAverage_Builder;

///////////////////////////////////////////////////////////////////////////////////////

FieldTimeAverage::FieldTimeAverage ( const std::string& name ) :
  common::Action(name),
  m_count(0)
{
  options().add("field", m_source_field)
    .pretty_name("Field")
    .description("The field to average")
    .attach_trigger(boost::bind(&FieldTimeAverage::trigger_field, this))
    .link_to(&m_source_field)
    .mark_basic();

  options().add("count", m_count)
    .pretty_name("count")
    .description("Numer of samples that were averaged so far")
    .link_to(&m_count);
}

void FieldTimeAverage::execute()
{
  if(is_null(m_source_field))
    throw common::SetupError(FromHere(), "No field configured for " + uri().path());

  const mesh::Field::ArrayT& source_array = m_source_field->array();
  mesh::Field::ArrayT& avg_array = m_statistics_field->array();
  const Uint nb_rows = m_source_field->size();
  const Uint row_size = m_source_field->row_size();

  for(Uint i = 0; i != nb_rows; ++i)
  {
    const Eigen::Map<RealVector const> source_row(&source_array[i][0], row_size);
    Eigen::Map<RealVector> avg_row(&avg_array[i][0], row_size);
    avg_row = (avg_row*static_cast<Real>(m_count) + source_row) / static_cast<Real>(m_count +1);
  }

  options().set("count", m_count+1u);
}

void FieldTimeAverage::trigger_field()
{
  if(is_null(m_source_field))
    return;
  const std::string new_field_name = std::string("average_") + m_source_field->name();
  mesh::Dictionary& dict = m_source_field->dict();
  m_statistics_field = Handle<mesh::Field>(dict.get_child(new_field_name));
  if(is_null(m_statistics_field))
  {
    m_statistics_field = dict.create_field(new_field_name, m_source_field->descriptor().description()).handle<mesh::Field>();
    m_statistics_field->descriptor().prefix_variable_names("avg_");
  }
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

