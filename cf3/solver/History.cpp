// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/BoostFilesystem.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/Builder.hpp"
#include "common/Signal.hpp"


#include "solver/History.hpp"

namespace cf3 {
namespace solver {

using namespace common;

common::ComponentBuilder < History , Component, LibSolver > History_Builder;

////////////////////////////////////////////////////////////////////////////////

History::History ( const std::string& name ) :
  Component(name)
{
  m_table_needs_resize = false;
  m_table = create_static_component< Table<Real> >("table");
  m_variables = create_static_component< math::VariablesDescriptor >("variables");

  options().add_option("dimension",0u);

  regist_signal ( "write" )
      .description( "Write history" )
      .pretty_name("Write" )
      .connect   ( boost::bind ( &History::signal_write,    this, _1 ) )
      .signature ( boost::bind ( &History::signature_write, this, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

void History::set(const std::string& var_name, const Real& var_value)
{
  if (properties().check(var_name) == false)
  {
    if (m_variables->nb_vars() == 0)
    {
      const Uint dim = options().option("dimension").value<Uint>();
      if ( dim == 0u)
        throw common::SetupError(FromHere(), "Dimension of "+uri().string()+" not set");
      m_variables->options().configure_option("dimension",dim);
    }

    m_variables->push_back(var_name,math::VariablesDescriptor::Dimensionalities::SCALAR);
    m_table_needs_resize = true;
  }
  properties()[var_name] = var_value;
}

////////////////////////////////////////////////////////////////////////////////

void History::set(const std::string& var_name, const std::vector<Real>& var_values)
{
  for (Uint i=0; i<var_values.size(); ++i)
  {
    set(var_name+"["+to_str(i)+"]",var_values[i]);
  }
//  if (properties().check(var_name) == false)
//  {
//    if (m_variables->nb_vars() == 0)
//    {
//      const Uint dim = options().option("dimension").value<Uint>();
//      if ( dim == 0u)
//        throw common::SetupError(FromHere(), "Dimension of "+uri().string()+" not set");
//      m_variables->options().configure_option("dimension",dim);
//    }

//    m_variables->push_back(var_name,var_values.size());
//    m_table_needs_resize = true;
//  }
//  properties()[var_name] = var_values;
}

////////////////////////////////////////////////////////////////////////////////

void History::save_entry()
{
  if (m_table_needs_resize)
  {
    if (is_not_null(m_buffer))
    {
      m_buffer->flush();
      m_buffer.reset();
    }

    m_table->set_row_size(m_variables->size());

    m_buffer = m_table->create_buffer_ptr();

    m_table_needs_resize = false;
  }

  std::vector<Real> row(m_variables->size());
  Uint v=0;
  for (Uint var_idx=0; var_idx<m_variables->nb_vars(); ++var_idx)
  {
    const Uint var_length = m_variables->var_length(var_idx);
    const Uint var_begin = m_variables->offset(var_idx);
    if (var_length == 1)
    {
      row[v++] = properties().value<Real>(m_variables->internal_variable_name(var_idx));
    }
    else
    {
      for (Uint i=0; i<var_length; ++i)
        row[v++] = properties().value<Real>(m_variables->internal_variable_name(var_idx)+"["+to_str(i)+"]");
    }
  }
  m_buffer->add_row(row);
}

////////////////////////////////////////////////////////////////////////////////

void History::flush()
{
  m_buffer->flush();
}

////////////////////////////////////////////////////////////////////////////////

const Handle<Table<Real> const> History::table()
{
  flush();
  return m_table;
}

////////////////////////////////////////////////////////////////////////////////

void History::signature_write(common::SignalArgs& args)
{
  SignalOptions opts(args);
  opts.add_option("file",URI("history.dat"))
      .description("Log file for history, will be overwritten!");
}

void History::signal_write(common::SignalArgs& args)
{
  if (PE::Comm::instance().rank()==0)
  {
    SignalOptions opts(args);
    URI file_uri = opts.option("file").value<URI>();

    boost::filesystem::fstream file;
    boost::filesystem::path path (file_uri.path());

    file.open(path,std::ios_base::out);
    if (!file) // didn't open so throw exception
    {
      throw boost::filesystem::filesystem_error( path.string() + " failed to open",
                                                 boost::system::error_code() );
    }

    // Write header, containing the variables
    // format: # var1 var2 vector[0] vector[1]
    file << "#";
    for (Uint var_idx=0; var_idx<m_variables->nb_vars(); ++var_idx)
    {
      const Uint var_length = m_variables->var_length(var_idx);
      if (var_length == 1)
      {
        file << "\t" << m_variables->user_variable_name(var_idx);
      }
      else
      {
        for (Uint i=0; i<var_length; ++i)
          file << "\t" << m_variables->user_variable_name(var_idx)<<"["<<i<<"]";
      }

    }
    file << "\n";

    flush();
    for (Uint row=0; row<m_table->size(); ++row)
    {
      for (Uint var_idx=0; var_idx<m_variables->nb_vars(); ++var_idx)
      {
        const Uint var_begin  = m_variables->offset(var_idx);
        const Uint var_length = m_variables->var_length(var_idx);
        for (Uint i=0; i<var_length; ++i)
          file << "\t" << (*m_table)[row][var_begin+i];
      }
      file << "\n";
    }

    file.close();
  }
}

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
