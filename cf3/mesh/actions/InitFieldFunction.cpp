// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Signal.hpp"
#include "common/XML/SignalOptions.hpp"
#include "math/AnalyticalFunction.hpp"

#include "mesh/actions/InitFieldFunction.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Region.hpp"
#include "mesh/Field.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;
  using namespace common::XML;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < InitFieldFunction, MeshTransformer, mesh::actions::LibActions> InitFieldFunction_Builder;

//////////////////////////////////////////////////////////////////////////////

InitFieldFunction::InitFieldFunction( const std::string& name )
: MeshTransformer(name)
{

  properties()["brief"] = std::string("Initialize a field");
  std::string desc;
  desc =
    "  Usage: InitFieldFunction vectorial function \n";
  properties()["description"] = desc;

  options().add("field", m_field)
      .description("Field to initialize")
      .pretty_name("Field")
      .link_to(&m_field)
      .mark_basic();

  options().add("cols",std::vector<Uint>())
      .pretty_name("columns")
      .description("field columns to consider. If empty, all columns are considered")
      .mark_basic();

  options().add("functions",std::vector<std::string>())
      .description("Functions to create of form 'func_var1 , func_var2 , func_var3'")
      .mark_basic();

  options().add("time",0.0).mark_basic();

  regist_signal ( "init_field" )
      .description( "Configure and execute" )
      .pretty_name("Initialize Field" )
      .connect   ( boost::bind ( &InitFieldFunction::signal_init_field,    this, _1 ) )
      .signature ( boost::bind ( &InitFieldFunction::signature_init_field, this, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

InitFieldFunction::~InitFieldFunction()
{
}

/////////////////////////////////////////////////////////////////////////////

void InitFieldFunction::config_function()
{
  m_function.functions( options()["functions"].value<std::vector<std::string> >() );
  m_function.parse();
}

////////////////////////////////////////////////////////////////////////////////

void InitFieldFunction::add_variable_with_suffix(std::vector<std::string>& names, std::vector< std::pair<std::string,std::string> >& replace, const std::string& name, const std::string& suffix)
{
  names.push_back(name+suffix);
  replace.push_back( std::make_pair(name+"["+suffix+"]",name+suffix) );
}

////////////////////////////////////////////////////////////////////////////////

void InitFieldFunction::execute()
{
  if (is_null(m_field))
    throw SetupError(FromHere(), "Option [field] was not set in ["+uri().path()+"]");

  Dictionary& dict = m_field->dict();


  std::vector<std::string> variable_names;
  std::vector< Handle<Field> > field_comps;
  std::vector< Uint > field_cols;
  std::vector< std::pair<std::string,std::string> > replace_var_names;

  boost_foreach( const Handle<Field>& field, dict.fields() )
  {
    for (Uint j=0; j<field->row_size(); ++j)
    {
      field_comps.push_back( field );
      field_cols.push_back( j );

      if ( field->has_tag(mesh::Tags::coordinates()) )
      {
        if      (j==0) variable_names.push_back("x");
        else if (j==1) variable_names.push_back("y");
        else if (j==2) variable_names.push_back("z");
      }
      else if ( field->var_type() == SCALAR )
      {
        variable_names.push_back(field->name());
      }
      else if ( field->var_type() == ARRAY )
      {
        add_variable_with_suffix(variable_names,replace_var_names,field->name(),to_str(j));
      }
      else if ( field->var_type() == VECTOR_2D || field->var_type() == VECTOR_3D)
      {
        if      (j==0) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"x");
        else if (j==1) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"y");
        else if (j==2) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"z");
      }
      else if ( field->var_type() == TENSOR_2D )
      {
        if      (j==0) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"xx");
        else if (j==1) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"xy");
        else if (j==2) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"yx");
        else if (j==3) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"yy");
      }
      else if ( field->var_type() == TENSOR_3D )
      {
        if      (j==0) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"xx");
        else if (j==1) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"xy");
        else if (j==2) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"xz");
        else if (j==3) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"yx");
        else if (j==4) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"yy");
        else if (j==5) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"yz");
        else if (j==6) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"zx");
        else if (j==7) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"zy");
        else if (j==8) add_variable_with_suffix(variable_names,replace_var_names,field->name(),"zz");
      }
    }
  }
  variable_names.push_back("t");

  // Create functions
  std::vector<Uint> cols = options().value< std::vector<Uint> >("cols");
  std::vector<std::string> option_functions = options().value< std::vector<std::string> >("functions");

  if (cols.empty())
  {
    cols.resize(option_functions.size());
    for (Uint f=0; f<option_functions.size(); ++f)
    {
      cols[f] = f;
    }
  }

  // Replace variablenames to be accepted by parser
  for (Uint f=0; f<option_functions.size(); ++f)
  {
    for (Uint v=0; v<replace_var_names.size(); ++v)
    {
      boost::algorithm::replace_all(option_functions[f],replace_var_names[v].first,replace_var_names[v].second);
    }
  }

  // create the functions
  std::vector<math::AnalyticalFunction> functions(option_functions.size());
  for (Uint f=0; f<functions.size(); ++f)
  {
    // check: columns must be of index smaller than index of field
    if (cols[f] >= m_field->row_size()) throw SetupError(FromHere(), "Specified column ["+to_str(cols[f])+"] doesn't exist. (field has only "+to_str(m_field->row_size())+" cols)");
    functions[f].parse(option_functions[f],variable_names);
  }

  std::vector<Real> constants;
  constants.push_back( options().value<Real>("time") );

  std::vector<Real> variables(variable_names.size());

  for (Uint pt=0; pt<dict.size(); ++pt)
  {
    // Assemble variables per point
    Uint c=0;
    for (Uint j=0; j<field_comps.size(); ++j, ++c)
    {
      variables[c] = field_comps[j]->array()[pt][field_cols[j]];
    }
    for (Uint j=0; j<constants.size(); ++j, ++c)
    {
      variables[c] = constants[j];
    }

    // Evaluate functions
    for (Uint f=0; f<cols.size(); ++f)
    {
      m_field->array()[pt][f] = functions[f](variables);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

void InitFieldFunction::signal_init_field( common::SignalArgs& args )
{
  SignalOptions opts( args );
  options().set("field",     opts.value< Handle<Field> >("field") );
  options().set("functions", opts.value< std::vector<std::string> >("functions") );
  options().set("cols",      opts.value< std::vector<Uint> >("cols") );
  execute();
}

////////////////////////////////////////////////////////////////////////////////

void InitFieldFunction::signature_init_field( common::SignalArgs& args )
{
  SignalOptions opts( args );
  opts.add("field",m_field).mark_basic();
  opts.add("functions",options().value< std::vector<std::string> >("functions")).mark_basic();
  opts.add("cols",options().value< std::vector<Uint> >("cols")).mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3
