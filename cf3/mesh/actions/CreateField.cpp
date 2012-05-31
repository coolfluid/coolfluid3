// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

#include "math/VariablesDescriptor.hpp"
#include "math/VectorialFunction.hpp"

#include "mesh/AInterpolator.hpp"
#include "mesh/LoadMesh.hpp"
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
      "    functions:array[string]=func_var1,func_var2_a, func_var2_b,func_var3\n"
      "\n"
      "  Optional arguments:\n"
      "    - dict:uri=./geometry\n";

  properties()["description"] = desc;

  options().add("name",std::string("new_field"))
      .description("Name of new field")
      .mark_basic();

  options().add("dict",URI("./"+std::string(mesh::Tags::geometry())))
      .description("Dictionary where field will be created in");

  options().add("variables",std::vector<std::string>())
      .description("Variable names and types of this field");

  options().add("functions",std::vector<std::string>())
      .description("");
}

/////////////////////////////////////////////////////////////////////////////

void CreateField::execute()
{
  // Check correct configuration
  Handle<Dictionary> dict ( m_mesh->access_component_checked( options().value<URI>("dict") ) );
  if (is_null(dict))
    throw SetupError(FromHere(), "Dictionary not found in "+m_mesh->uri().string());

  std::vector<std::string> function_str = options().value< std::vector<std::string> >("functions");

  std::vector<std::string> variables_opt = options().value< std::vector<std::string> >("variables");
  std::string var_description;
  if (variables_opt.empty())
    var_description = options().value<std::string>("name")+"["+to_str(function_str.size())+"]";
  else
  {
    for(Uint i=0; i<variables_opt.size(); ++i)
    {
      if (i!=0) var_description += ",";
      var_description += variables_opt[i];
    }
  }
  Field& new_field = dict->create_field(options().value<std::string>("name"), var_description);

  if (new_field.row_size()!=function_str.size())
  {
    std::stringstream ss;
    ss << "Number of variables and functions doesn't match:\n";
    ss << "variables = " << var_description << "\n";
    ss << "functions = \n";
    boost_foreach(const std::string& function, function_str)
        ss << "    " << function << "\n";
    throw SetupError(FromHere(),ss.str());
  }


  std::vector< std::string >  var_names;
  std::vector< std::string >  mod_var_names;
  std::vector< Table<Real>::ArrayT const* > var_arrays;
  std::vector< Uint >         var_array_idx;

  std::string var_name;
  std::string mod_var_name;

  boost_foreach ( Field& field, find_components_recursively<Field>(*dict))
  {
//    Uint count=0;
//    for (Uint v=0; v<field.nb_vars(); ++v)
//    {
//      std::string var_name = field.var_name(v);
//      for (Uint i=0; i<field.var_length(v); ++i)
//      {
//        replace_var_name( var_name+"["+to_str(i)+"]"  , "r7ys5z["+to_str(count)+"]"  ,function_str);
//        switch (i) {
//          case XX: replace_var_name( var_name+"x" , "r7ys5z["+to_str(count)+"]"  ,function_str); break;
//          case YY: replace_var_name( var_name+"y" , "r7ys5z["+to_str(count)+"]"  ,function_str); break;
//          case ZZ: replace_var_name( var_name+"z" , "r7ys5z["+to_str(count)+"]"  ,function_str); break;
//        }
//        ++count;
//      }
//      if (field.var_length(v) == 1)
//        replace_var_name( var_name , "r7ys5z["+to_str(count-1)+"]"  ,function_str);
//      replace_var_name( "r7ys5z" , field.name()  ,function_str);
//    }

    for (Uint v=0; v<field.row_size(); ++v)
    {
      var_name    =field.name()+"["+to_str(v)+"]";
      mod_var_name=field.name()+"_"+to_str(v)+"_";

      for (Uint s=0; s<function_str.size(); ++s)
        boost::algorithm::replace_all(function_str[s],var_name,mod_var_name);
      var_names.push_back(mod_var_name);
      var_arrays.push_back(&field.array());
      var_array_idx.push_back(v);
    }
  }


  // parse function

  VectorialFunction vectorial_function;
  vectorial_function.functions(function_str);
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
}

////////////////////////////////////////////////////////////////////////////////

void CreateField::replace_var_name(const std::string& var_from, const std::string& var_to, std::vector<std::string>& functions)
{
  for (Uint s=0; s<functions.size(); ++s)
    boost::algorithm::replace_all(functions[s],var_from,var_to);
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3
