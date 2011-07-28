// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>

#include "Common/CBuilder.hpp"
#include "Common/CLink.hpp"
#include "Common/Foreach.hpp"
#include "Common/FindComponents.hpp"
#include "Common/OptionT.hpp"
#include "Common/StringConversion.hpp"
#include "Common/Signal.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/LibMesh.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/FieldGroup.hpp"
#include "Mesh/CMeshElements.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/WriteMesh.hpp"
#include "Mesh/MeshMetadata.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CFaces.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::XML;

Common::ComponentBuilder < CMesh, Component, LibMesh > CMesh_Builder;

////////////////////////////////////////////////////////////////////////////////

CMesh::CMesh ( const std::string& name  ) :
  Component ( name ),
  m_dimension(0u),
  m_dimensionality(0u)
{
  mark_basic(); // by default meshes are visible

  m_properties.add_property("nb_cells",Uint(0));
  m_properties.add_property("nb_faces",Uint(0));
  m_properties.add_property("nb_nodes",Uint(0));
  m_properties.add_property("dimensionality",Uint(0));
  m_properties.add_property("dimension",Uint(0));

  m_elements   = create_static_component_ptr<CMeshElements>("elements");
  m_topology   = create_static_component_ptr<CRegion>("topology");
  m_metadata   = create_static_component_ptr<MeshMetadata>("metadata");

  regist_signal ( "write_mesh" )
      ->description( "Write mesh, guessing automatically the format" )
      ->pretty_name("Write Mesh" )
      ->connect   ( boost::bind ( &CMesh::signal_write_mesh,    this, _1 ) )
      ->signature ( boost::bind ( &CMesh::signature_write_mesh, this, _1 ) );

  m_nodes = create_static_component_ptr<CNodes>(Mesh::Tags::nodes());
  m_nodes->add_tag(Mesh::Tags::nodes());

}

////////////////////////////////////////////////////////////////////////////////

CMesh::~CMesh()
{
}

////////////////////////////////////////////////////////////////////////////////

void CMesh::initialize_nodes(const Uint nb_nodes, const Uint dimension)
{
  cf_assert(dimension > 0);

  nodes().configure_option("type",    FieldGroup::Basis::to_str(FieldGroup::Basis::POINT_BASED));
  nodes().configure_option("space",   CEntities::MeshSpaces::to_str(CEntities::MeshSpaces::MESH_NODES));
  nodes().configure_option("topology",topology().uri());
  nodes().coordinates().configure_option("var_names",std::vector<std::string>(1,std::string("coord")));
  nodes().coordinates().configure_option("var_types",std::vector<std::string>(1,to_str(dimension)));
  nodes().resize(nb_nodes);

  cf_assert(nodes().size() == nb_nodes);
  cf_assert(nodes().coordinates().row_size() == dimension);
  m_dimension = dimension;
  property("dimension") = m_dimension;
  property("nb_nodes")  = nodes().size();
}

////////////////////////////////////////////////////////////////////////////////

void CMesh::update_statistics()
{
  cf_assert(m_dimension == nodes().coordinates().row_size() );
  boost_foreach ( CEntities& elements, find_components_recursively<CEntities>(topology()) )
    m_dimensionality = std::max(m_dimensionality,elements.element_type().dimensionality());

  Uint nb_cells = 0;
  boost_foreach ( CCells& elements, find_components_recursively<CCells>(topology()) )
    nb_cells += elements.size();

  Uint nb_faces = 0;
  boost_foreach ( CFaces& elements, find_components_recursively<CFaces>(topology()) )
    nb_faces += elements.size();

  property("dimension") = m_dimension;
  property("dimensionality")= m_dimensionality;
  property("nb_cells") = nb_cells;
  property("nb_faces") = nb_faces;
  property("nb_nodes") = nodes().size();
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup& CMesh::create_field_group( const std::string& name,
                                       const FieldGroup::Basis::Type base,
                                       const std::string& space )
{
  return create_field_group ( name, base, space, topology() );
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup& CMesh::create_field_group( const std::string& name,
                                       const FieldGroup::Basis::Type base,
                                       const std::string& space,
                                       const CRegion& topology )
{
  FieldGroup& field_group = create_component<FieldGroup>(name);
  field_group.configure_option("type",FieldGroup::Basis::to_str(base));
  field_group.configure_option("space",space);
  field_group.configure_option("topology",topology.uri());
  return field_group;
}

////////////////////////////////////////////////////////////////////////////////

CField& CMesh::create_field( const std::string& name ,
                             const CField::Basis::Type base,
                             const std::string& space,
                             const std::string& variables)
{
  std::vector<std::string> tokenized_variables(0);

  if (variables == "scalar_same_name")
  {
    tokenized_variables.push_back(name+"[scalar]");
  }
  else
  {
    typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
    boost::char_separator<char> sep(",");
    Tokenizer tokens(variables, sep);

    for (Tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
      tokenized_variables.push_back(*tok_iter);
  }


  std::vector<std::string> names;
  std::vector<std::string> types;
  BOOST_FOREACH(std::string var, tokenized_variables)
  {
    boost::regex e_variable("([[:word:]]+)?[[:space:]]*\\[[[:space:]]*([[:word:]]+)[[:space:]]*\\]");

    boost::match_results<std::string::const_iterator> what;
    if (regex_search(var,what,e_variable))
    {
      names.push_back(what[1]);
      types.push_back(what[2]);
    }
    else
      throw ShouldNotBeHere(FromHere(), "No match found for VarType " + var);
  }

  CField& field = *create_component_ptr<CField>(name);
  field.set_topology(topology());
  field.configure_option("Space",space);
  field.configure_option("VarNames",names);
  field.configure_option("VarTypes",types);
  field.configure_option("FieldType",CField::Basis::Convert::instance().to_str(base));
  field.create_data_storage();

  return field;
}

////////////////////////////////////////////////////////////////////////////////

CField& CMesh::create_scalar_field( const std::string& name , CField& based_on_field)
{
  CField& field = *create_component_ptr<CField>(name);
  field.set_topology(based_on_field.topology());

  std::vector<std::string> names(1,name);
  field.configure_option("VarNames",names);

  std::vector<std::string> types(1,"scalar");
  field.configure_option("VarTypes",types);

  std::string base;   based_on_field.option("FieldType").put_value(base);
  field.configure_option("FieldType",base);

  std::string space; based_on_field.option("Space").put_value(space);
  field.configure_option("Space",space);

  field.create_data_storage();

  return field;

}

////////////////////////////////////////////////////////////////////////////////

CField& CMesh::create_field( const std::string& name , CField& based_on_field)
{
  CField& field = *create_component_ptr<CField>(name);
  field.set_topology(based_on_field.topology());

  std::vector<std::string> names; based_on_field.option("VarNames").put_value(names);

  for (Uint i=0; i<names.size(); ++i)
    names[i] = name+"["+to_str(i)+"]";
  field.configure_option("VarNames",names);

  std::vector<std::string> types; based_on_field.option("VarTypes").put_value(types);
  field.configure_option("VarTypes",types);

  std::string base;   based_on_field.option("FieldType").put_value(base);
  field.configure_option("FieldType",base);

  std::string space; based_on_field.option("Space").put_value(space);
  field.configure_option("Space",space);

  field.create_data_storage();
  return field;

}
////////////////////////////////////////////////////////////////////////////////

CField& CMesh::create_scalar_field(const std::string& field_name, const std::string& variable_name, const CF::Mesh::CField::Basis::Type base)
{
  const std::vector<std::string> names(1, variable_name);
  const std::vector< CField::VarType > types(1, CField::SCALAR);
  return create_field(field_name, base, names, types);
}


////////////////////////////////////////////////////////////////////////////////

CField& CMesh::create_field(const std::string& name, const CField::Basis::Type base, const std::vector< std::string >& variable_names, const std::vector< CField::VarType > variable_types)
{
  cf_assert(variable_names.size() == variable_types.size());

  /// @todo Treat variable_types using EnumT, and store enum values in the option instead of strings
  std::vector<std::string> types_str;
  types_str.reserve( variable_types.size() );
  boost_foreach(const CField::VarType var_type, variable_types)
  {
    types_str.push_back( boost::lexical_cast<std::string>(var_type) );
  }

  CField& field = *create_component_ptr<CField>(name);
  field.set_topology(topology());
  field.configure_option("VarNames",variable_names);
  field.configure_option("VarTypes",types_str);
  field.configure_option("FieldType", CField::Basis::Convert::instance().to_str(base) );
  field.create_data_storage();

  return field;
}

////////////////////////////////////////////////////////////////////////////////

CNodes& CMesh::nodes()
{
  return *m_nodes;
}

////////////////////////////////////////////////////////////////////////////////

const CNodes& CMesh::nodes() const
{
  return *m_nodes;
}

////////////////////////////////////////////////////////////////////////////////

CMeshElements& CMesh::elements()
{
  return *m_elements;
}

////////////////////////////////////////////////////////////////////////////////

const CMeshElements& CMesh::elements() const
{
  return *m_elements;
}

//////////////////////////////////////////////////////////////////////////////

void CMesh::signature_write_mesh ( SignalArgs& node)
{
  SignalOptions options( node );

  options.add_option< OptionT<std::string> >("file" , name() + ".msh" )
      ->description("File to write" );

  boost_foreach (CField& field, find_components<CField>(*this))
  {
    options.add_option< OptionT<bool> >(field.name(), false )
        ->description("Mark if field gets to be written");
  }
}

////////////////////////////////////////////////////////////////////////////////

void CMesh::signal_write_mesh ( SignalArgs& node )
{
  WriteMesh::Ptr mesh_writer = create_component_ptr<WriteMesh>("writer");

  SignalOptions options( node );

  std::string file = name()+".msh";

  if (options.check("file"))
    file = options.value<std::string>("file");

  // check protocol for file loading
  // if( file.scheme() != URI::Scheme::FILE )
  //   throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + file.string() + "\'" );

  URI fpath( file );
//  if( fpath.scheme() != URI::Scheme::FILE )
//    throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + fpath.string() + "\'" );

  std::vector<URI> fields;

  boost_foreach( CField& field, find_components<CField>(*this))
  {
    if (options.check(field.name()))
    {
      bool add_field = options.value<bool>( field.name() );
      if (add_field)
        fields.push_back(field.uri());
    }
  }
  mesh_writer->write_mesh(*this,fpath,fields);
  remove_component(mesh_writer->name());
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
