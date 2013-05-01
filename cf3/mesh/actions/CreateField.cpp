// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/StringConversion.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/XML/SignalOptions.hpp"
#include "common/Signal.hpp"

#include "math/VariablesDescriptor.hpp"
#include "math/VectorialFunction.hpp"

#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"

#include "mesh/actions/CreateField.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;
  using namespace math;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CreateField, MeshTransformer, mesh::actions::LibActions> CreateField_Builder;

//////////////////////////////////////////////////////////////////////////////

CreateField::CreateField( const std::string& name )
: MeshTransformer(name)
{
  properties()["brief"] = std::string("CreateField mesh");
  std::string desc;
  desc =
      "  Usage: CreateField \n\n"
      "    name:string=new_field \n"
      "    variables:string=var1[scalar],var2[vector],var3[scalar]\n"
      "    functions:array[string]='var_1=func_var1 , var2_a=func_var2_a , var2_b=func_var2_b , var3=func_var3'\n"
      "\n"
      "  Optional arguments:\n"
      "    - dict:uri=./geometry\n";

  properties()["description"] = desc;

  options().add("name",std::string("new_field"))
      .description("Name of new field")
      .mark_basic();

  options().add("dict",URI("./"+std::string(mesh::Tags::geometry())))
      .description("Dictionary where field will be created in")
      .mark_basic();

  options().add("functions",std::vector<std::string>())
      .description("Functions to create of form 'var_1=func_var1 , var2_a=func_var2_a , var2_b=func_var2_b , var3=func_var3'")
      .mark_basic();

  regist_signal ( "create_field" )
      .description( "Create a field in a given dictionary" )
      .pretty_name("Create Field" )
      .connect   ( boost::bind ( &CreateField::signal_create_field,    this, _1 ) )
      .signature ( boost::bind ( &CreateField::signature_create_field, this, _1 ) );

}

/////////////////////////////////////////////////////////////////////////////

std::vector<std::string> CreateField::split_function_list(const std::string& str)
{
  std::string trimmed_str(str);
  boost::algorithm::trim(trimmed_str);
  boost::algorithm::trim_left_if (trimmed_str,boost::algorithm::is_any_of("["));
  boost::algorithm::trim_right_if(trimmed_str,boost::algorithm::is_any_of("]"));
  std::vector<std::string> split;
  Uint scope_level = 0;
  Uint start_pos = 0;
  Uint pos = 0;
  boost_foreach(const char& c, trimmed_str)
  {
    if (c == '(') ++scope_level;
    if (c == ')') --scope_level;
    if (c == ',' && scope_level == 0)
    {
      split.push_back( std::string(trimmed_str.begin()+start_pos,trimmed_str.begin()+pos) );
      start_pos = pos + 1;
    }
    ++pos;
  }
  split.push_back( std::string(trimmed_str.begin()+start_pos,trimmed_str.end()) );
  return split;
}

////////////////////////////////////////////////////////////////////////////////

void CreateField::execute()
{
  // Check correct configuration
  if ( is_null(m_mesh) ) throw SetupError(FromHere(), "mesh is not configured");
  Handle<Dictionary> dict ( m_mesh->access_component_checked( options().value<URI>("dict") ) );
  if (is_null(dict))
    throw SetupError(FromHere(), "Dictionary not found in "+m_mesh->uri().string());

  std::vector<std::string> functions_str = options().value< std::vector<std::string> >("functions");

  create_field(options().value<std::string>("name"),*dict,functions_str);
}

////////////////////////////////////////////////////////////////////////////////

Handle<Field> CreateField::create_field(const std::string& name, Dictionary& dict, const std::vector<std::string>& _functions)
{
  std::vector<std::string> functions_str=_functions;
  std::vector<std::string> variables_str(functions_str.size());

  for (Uint f=0; f<functions_str.size(); ++f)
  {
    std::string str = functions_str[f];
    std::vector<std::string> split;
    boost::split(split, str, boost::is_any_of("="));

    variables_str[f]  = split[0];
    functions_str[f]  = split[1];
  }

  std::string var_description;
  for(Uint i=0; i<variables_str.size(); ++i)
  {
    if (i!=0) var_description += ",";
    var_description += variables_str[i];
  }

  Field& new_field = dict.create_field(name, var_description);

  std::vector<std::string> functions;
  boost_foreach(const std::string& function_list, functions_str)
  {
    std::vector<std::string> split_functions = split_function_list(function_list);
    boost_foreach(const std::string& f_str, split_functions)
    {
      functions.push_back(f_str);
    }
  }

  if (new_field.row_size()!=functions.size())
  {
    std::stringstream ss;
    ss << "Number of variables and functions doesn't match:\n";
    ss << "variables = " << var_description << "\n";
    ss << "functions = \n";
    boost_foreach(const std::string& function, functions)
        ss << "    " << function << "\n";
    throw SetupError(FromHere(),ss.str());
  }

  std::vector< std::string >  var_names;
  std::vector< std::string >  mod_var_names;
  std::vector< Table<Real>::ArrayT const* > var_arrays;
  std::vector< Uint >         var_array_idx;

  std::string var_name;
  std::string mod_var_name;

  boost_foreach ( Field& field, find_components_recursively<Field>(dict))
  {
    for (Uint v=0; v<field.row_size(); ++v)
    {
      var_name    =field.name()+"["+to_str(v)+"]";
      mod_var_name=field.name()+"_"+to_str(v)+"_";

      for (Uint s=0; s<functions.size(); ++s)
        boost::algorithm::replace_all(functions[s],var_name,mod_var_name);
      var_names.push_back(mod_var_name);
      var_arrays.push_back(&field.array());
      var_array_idx.push_back(v);
    }
  }

  // Add the x, y, z coordinates as aliases
  const Field& coordinates = dict.coordinates();
  std::vector<std::string> xyz(3);
  xyz[0] = "x";
  xyz[1] = "y";
  xyz[2] = "z";
  for (Uint d=0; d<coordinates.row_size(); ++d)
  {
    var_names.push_back(xyz[d]);
    var_arrays.push_back(&coordinates.array());
    var_array_idx.push_back(d);
  }


  // parse function

  VectorialFunction vectorial_function;
  vectorial_function.functions(functions);
  vectorial_function.variables(var_names);
  vectorial_function.parse();


  // Evaluate function

  std::vector<Real> params(var_names.size());
  for (Uint pt=0; pt<new_field.size(); ++pt)
  {
    for (Uint v=0; v<params.size(); ++v)
    {
      params[v] = (*var_arrays[v])[pt][var_array_idx[v]];
    }
    Table<Real>::ArrayT::reference ref = new_field.array()[pt];
    vectorial_function.evaluate(params,ref);
  }

  return new_field.handle<Field>();
}

////////////////////////////////////////////////////////////////////////////////

void CreateField::replace_var_name(const std::string& var_from, const std::string& var_to, std::vector<std::string>& functions)
{
  for (Uint s=0; s<functions.size(); ++s)
    boost::algorithm::replace_all(functions[s],var_from,var_to);
}

////////////////////////////////////////////////////////////////////////////////

void CreateField::signal_create_field( common::SignalArgs& args)
{
  common::XML::SignalOptions options(args);
  Handle<Dictionary> dict ( access_component_checked( options.value<URI>("dict") ) );

  std::vector<std::string> functions_str = options.value< std::vector<std::string> >("functions");

  Handle<Field> created_component = create_field(options.value<std::string>("name"),*dict,functions_str);
  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", created_component->uri());
}

////////////////////////////////////////////////////////////////////////////////

void CreateField::signature_create_field( common::SignalArgs& args)
{
  common::XML::SignalOptions options(args);
  options.add("name",std::string("new_field")).mark_basic();
  options.add("dict",URI("./geometry")).mark_basic();
  options.add("functions", std::vector<std::string>() ).mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3
