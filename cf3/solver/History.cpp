// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

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

  options().add("dimension",0u).mark_basic();


  m_logging = true;
  options().add("logging",m_logging)
      .description("Turn on logging at every entry")
      .link_to(&m_logging);

  // Extension TSV for "Tab Separated Values"
  options().add("file",URI("history.tsv"))
      .description("Log file for history").mark_basic();

  regist_signal ( "write" )
      .description( "Write history" )
      .pretty_name("Write" )
      .connect   ( boost::bind ( &History::signal_write,    this, _1 ) )
      .signature ( boost::bind ( &History::signature_write, this, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

History::~History()
{
  if (m_file)
  {
    m_file.close();
  }
}

////////////////////////////////////////////////////////////////////////////////

void History::set(const std::string& var_name, const Real& var_value)
{
  if (properties().check(var_name) == false)
  {
    if (m_variables->nb_vars() == 0)
    {
      const Uint dim = options().value<Uint>("dimension");
      if ( dim == 0u)
        throw common::SetupError(FromHere(), "Dimension of "+uri().string()+" not set");
      m_variables->options().set("dimension",dim);
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
}

////////////////////////////////////////////////////////////////////////////////

bool History::resize_if_necessary()
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
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

void History::save_entry()
{

  const HistoryEntry this_entry = entry();

  bool resized = resize_if_necessary();
  m_buffer->add_row(this_entry.data());

  if (m_logging)
  {
    if (PE::Comm::instance().rank() == 0)
    {
      if (resized)
        m_file.close();

      if (!m_file)
      {
        flush();
        open_file(m_file,options().value<URI>("file"));
        write_file(m_file);
        m_file.flush();
      }
      else
      {
        m_file << this_entry << "\n";
        m_file.flush();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void History::flush()
{
  if(is_not_null(m_buffer))
    m_buffer->flush();
}

////////////////////////////////////////////////////////////////////////////////

Handle<Table<Real> const> History::table()
{
  flush();
  return m_table;
}

////////////////////////////////////////////////////////////////////////////////

Handle<math::VariablesDescriptor const> History::variables() const
{
  return m_variables;
}

////////////////////////////////////////////////////////////////////////////////

void History::signature_write(common::SignalArgs& args)
{
  SignalOptions opts(args);
  opts.add("file",URI("history.tsv"))
      .description("Tab Separated Value log file for output of history");
}

////////////////////////////////////////////////////////////////////////////////

void History::signal_write(common::SignalArgs& args)
{
  if (PE::Comm::instance().rank()==0)
  {
    SignalOptions opts(args);
    URI file_uri = opts.option("file").value<URI>();

    boost::filesystem::fstream file;
    open_file(file,file_uri);
    write_file(file);

    file.close();
  }
}

////////////////////////////////////////////////////////////////////////////////

void History::open_file(boost::filesystem::fstream& file, const common::URI& file_uri)
{
  boost::filesystem::path path (file_uri.path());
  file.open(path,std::ios_base::out);
  if (!file) // didn't open so throw exception
  {
    throw boost::filesystem::filesystem_error( path.string() + " failed to open",
                                               boost::system::error_code() );
  }
  file.precision(10);
}

////////////////////////////////////////////////////////////////////////////////

std::string History::file_header() const
{
  std::stringstream ss;

  ss << "#";
  for (Uint var_idx=0; var_idx<m_variables->nb_vars(); ++var_idx)
  {
    const Uint var_length = m_variables->var_length(var_idx);
    if (var_length == 1)
    {
      ss << "\t" << std::setw(16) << m_variables->user_variable_name(var_idx);
    }
    else
    {
      for (Uint i=0; i<var_length; ++i)
        ss << "\t" << std::setw(16) << m_variables->user_variable_name(var_idx)<<"["<<i<<"]";
    }

  }
  ss << "\n";
  return ss.str();
}

////////////////////////////////////////////////////////////////////////////////

void History::write_file(boost::filesystem::fstream& file)
{
  // Write header, containing the variables
  // format: # var1 var2 vector[0] vector[1]
  file << file_header();

  flush();
  for (Uint row=0; row<m_table->size(); ++row)
  {
    for (Uint var_idx=0; var_idx<m_variables->nb_vars(); ++var_idx)
    {
      const Uint var_begin  = m_variables->offset(var_idx);
      const Uint var_length = m_variables->var_length(var_idx);
      for (Uint i=0; i<var_length; ++i)
        file << "\t" <<std::scientific << std::setw(16) << (*m_table)[row][var_begin+i];
    }
    file << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////////

HistoryEntry History::entry() const
{
  return HistoryEntry(*this);
}

////////////////////////////////////////////////////////////////////////////////

HistoryEntry::HistoryEntry(const History& history)
  : m_history(history)
{
  const math::VariablesDescriptor& vars = *variables();
  m_entry.resize(vars.size());
  Uint v=0;
  for (Uint var_idx=0; var_idx<vars.nb_vars(); ++var_idx)
  {
    const Uint var_length = vars.var_length(var_idx);
    const Uint var_begin  = vars.offset(var_idx);
    if (var_length == 1)
    {
      m_entry[v++] = m_history.properties().value<Real>(vars.internal_variable_name(var_idx));
    }
    else
    {
      for (Uint i=0; i<var_length; ++i)
        m_entry[v++] = m_history.properties().value<Real>(vars.internal_variable_name(var_idx)+"["+to_str(i)+"]");
    }
  }
 }

////////////////////////////////////////////////////////////////////////////////

Handle<math::VariablesDescriptor const> HistoryEntry::variables() const
{
  return m_history.variables();
}

////////////////////////////////////////////////////////////////////////////////

const std::vector<Real>& HistoryEntry::data() const
{
  return m_entry;
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<< ( std::ostream& os, const HistoryEntry& history_entry )
{
  for (Uint i=0; i<history_entry.m_entry.size(); ++i)
    os << "\t" <<  std::scientific << std::setw(16) << history_entry.m_entry[i];
  return os;
}

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
