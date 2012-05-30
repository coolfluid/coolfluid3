// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/StringConversion.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/AInterpolator.hpp"
#include "mesh/LoadMesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"

#include "mesh/actions/ImportVariables.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;
  
////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ImportVariables, MeshTransformer, mesh::actions::LibActions> ImportVariables_Builder;

//////////////////////////////////////////////////////////////////////////////

ImportVariables::ImportVariables( const std::string& name )
: MeshTransformer(name)
{

  properties()["brief"] = std::string("ImportVariables mesh");
  std::string desc;
  desc =
      "  Usage: ImportVariables \n\n"
      "    files:array[uri]=file_P0.msh, file_P1.msh"
      "    variables:array[string]=var1,var2,var3\n"
      "\n"
      "  Optional arguments:\n"
      "    - dict:uri=./geometry\n";

  properties()["description"] = desc;

  options().add("files",std::vector<URI>())
      .description("Files to import mesh from")
      .mark_basic();

  options().add("variables",m_import_var_names).link_to(&m_import_var_names)
      .description("Variables to import from other file")
      .mark_basic();

  options().add("dict",URI("./"+std::string(mesh::Tags::geometry())))
      .description("Standard location where variables will be imported into");

  options().add("variable_names",std::vector<std::string>())
      .description("Names to give to imported variables");

  m_mesh_loader  = create_static_component<LoadMesh>("mesh_loader");

//  options().add("interpolator",std::string("cf3.mesh.ShapeFunctionInterpolator"));
  options().add("interpolator",std::string("cf3.mesh.PseudoLaplacianLinearInterpolator"));
  m_interpolator = create_component<AInterpolator>("interpolator",options().value<std::string>("interpolator"));
}

/////////////////////////////////////////////////////////////////////////////

void ImportVariables::execute()
{
  // Check correct configuration
  Handle<Dictionary> dict ( m_mesh->access_component_checked( options().value<URI>("dict") ) );
  if (is_null(dict))
    throw SetupError(FromHere(), "Dictionary not found in "+m_mesh->uri().string());

  std::vector< std::string > var_names = options().value< std::vector<std::string> >("variable_names");
  if (var_names.size() == 0)
    var_names = m_import_var_names;
  if ( var_names.size() != m_import_var_names.size() )
    throw SetupError(FromHere(), "Variable names has different size than number of variables to import");

  if (options().value< std::vector<URI> >("files").size() == 0)
    throw SetupError(FromHere(), "No files have been configured to load variables from");

  // Import mesh
  boost::shared_ptr<Mesh> loaded_mesh = allocate_component<Mesh>("imported");
  m_mesh_loader->load_multiple_files( options().value< std::vector<URI> >("files") , *loaded_mesh );

  m_interpolator->options().set("store",true);

  if (m_import_var_names.size())
  {
    // Find the variables
    for (Uint v=0; v<var_names.size(); ++v)
    {
      std::string import_var_name = m_import_var_names[v];
      std::string var_name = var_names[v];
      boost_foreach (const Handle<Dictionary>& loaded_dict, loaded_mesh->dictionaries() )
      {
        boost_foreach (const Handle<Field>& loaded_field, loaded_dict->fields())
        {
          if (loaded_field->has_variable(import_var_name))
          {
            std::string var_dimensionality = math::VariablesDescriptor::Dimensionalities::Convert::instance().to_str( loaded_field->descriptor().dimensionality(import_var_name) );
            Uint var_idx = loaded_field->var_offset(import_var_name);
            Uint var_length = loaded_field->var_length(import_var_name);
            std::vector<Uint> source_vars(var_length);
            std::vector<Uint> target_vars(var_length);
            for (Uint i=0; i<var_length; ++i)
            {
              source_vars[i] = var_idx+i;
              target_vars[i] = i;
            }
            Field& imported_field = dict->create_field(var_name, var_name+"["+var_dimensionality+"]");
            m_interpolator->interpolate_vars(*loaded_field,imported_field,source_vars,target_vars);
          }
        }
      }
    }
  }
  else
  {
    boost_foreach (const Handle<Dictionary>& loaded_dict, loaded_mesh->dictionaries() )
    {
      boost_foreach (const Handle<Field>& loaded_field, loaded_dict->fields())
      {
        for (Uint v=0; v<loaded_field->nb_vars(); ++v)
        {

          std::string import_var_name = loaded_field->var_name(v);
          std::string var_name = import_var_name;

          std::string var_dimensionality = math::VariablesDescriptor::Dimensionalities::Convert::instance().to_str( loaded_field->descriptor().dimensionality(import_var_name) );
          Uint var_idx = loaded_field->var_offset(import_var_name);
          Uint var_length = loaded_field->var_length(import_var_name);
          std::vector<Uint> source_vars(var_length);
          std::vector<Uint> target_vars(var_length);
          for (Uint i=0; i<var_length; ++i)
          {
            source_vars[i] = var_idx+i;
            target_vars[i] = i;
          }
          Field& imported_field = dict->create_field(var_name, var_name+"["+var_dimensionality+"]");
          m_interpolator->interpolate_vars(*loaded_field,imported_field,source_vars,target_vars);
        }
      }
    }

  }
}

//////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3
