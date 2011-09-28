// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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
#include "Common/Tags.hpp"

#include "Math/VariablesDescriptor.hpp"

#include "Mesh/LibMesh.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/FieldGroup.hpp"
#include "Mesh/CMeshElements.hpp"
#include "Mesh/ElementType.hpp"
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
  m_properties.add_property(Common::Tags::dimension(),Uint(0));

  m_elements   = create_static_component_ptr<CMeshElements>("elements");
  m_topology   = create_static_component_ptr<CRegion>("topology");
  m_metadata   = create_static_component_ptr<MeshMetadata>("metadata");

  regist_signal ( "write_mesh" )
      ->description( "Write mesh, guessing automatically the format" )
      ->pretty_name("Write Mesh" )
      ->connect   ( boost::bind ( &CMesh::signal_write_mesh,    this, _1 ) )
      ->signature ( boost::bind ( &CMesh::signature_write_mesh, this, _1 ) );

  m_nodes = create_static_component_ptr<Geometry>(Mesh::Tags::nodes());
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

  geometry().configure_option("type",    FieldGroup::Basis::to_str(FieldGroup::Basis::POINT_BASED));
  geometry().configure_option("space",   std::string(Tags::geometry()));
  geometry().configure_option("topology",topology().uri());

  geometry().coordinates().set_field_group(geometry());
  geometry().coordinates().set_topology(geometry().topology());
  geometry().coordinates().set_basis(FieldGroup::Basis::POINT_BASED);
  geometry().coordinates().descriptor().configure_option(Common::Tags::dimension(),dimension);  geometry().resize(nb_nodes);

  cf_assert(geometry().size() == nb_nodes);
  cf_assert(geometry().coordinates().row_size() == dimension);
  m_dimension = dimension;
  property(Common::Tags::dimension()) = m_dimension;
  property("nb_nodes")  = geometry().size();
}

////////////////////////////////////////////////////////////////////////////////

void CMesh::update_statistics()
{
  cf_assert(m_dimension == geometry().coordinates().row_size() );
  boost_foreach ( CEntities& elements, find_components_recursively<CEntities>(topology()) )
    m_dimensionality = std::max(m_dimensionality,elements.element_type().dimensionality());

  Uint nb_cells = 0;
  boost_foreach ( CCells& elements, find_components_recursively<CCells>(topology()) )
    nb_cells += elements.size();

  Uint nb_faces = 0;
  boost_foreach ( CFaces& elements, find_components_recursively<CFaces>(topology()) )
    nb_faces += elements.size();

  property(Common::Tags::dimension()) = m_dimension;
  property("dimensionality")= m_dimensionality;
  property("nb_cells") = nb_cells;
  property("nb_faces") = nb_faces;
  property("nb_nodes") = geometry().size();
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup& CMesh::create_field_group( const std::string& name,
                                       const FieldGroup::Basis::Type base )
{
  return create_field_group ( name, base, name, topology() );
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

void CMesh::create_space( const std::string& name, const FieldGroup::Basis::Type base, const std::string& space_lib_name)
{
  create_space(name,base,space_lib_name,topology());
}

////////////////////////////////////////////////////////////////////////////////

void CMesh::create_space( const std::string& name, const FieldGroup::Basis::Type base, const std::string& space_lib_name, CRegion& topology)
{
  switch (base)
  {
  case FieldGroup::Basis::POINT_BASED:
  case FieldGroup::Basis::ELEMENT_BASED:
    boost_foreach(CEntities& elements, find_components_recursively<CEntities>(topology))
      elements.create_space(name,space_lib_name+"."+elements.element_type().shape_name());
    break;
  case FieldGroup::Basis::CELL_BASED:
    boost_foreach(CCells& elements, find_components_recursively<CCells>(topology))
      elements.create_space(name,space_lib_name+"."+elements.element_type().shape_name());
    break;
  case FieldGroup::Basis::FACE_BASED:
    boost_foreach(CEntities& elements, find_components_recursively_with_tag<CEntities>(topology,Mesh::Tags::face_entity()))
      elements.create_space(name,space_lib_name+"."+elements.element_type().shape_name());
    break;
  case FieldGroup::Basis::INVALID:
  default:
    throw BadValue(FromHere(),"value "+FieldGroup::Basis::to_str(base)+" not supported for base");
  }
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup& CMesh::create_space_and_field_group( const std::string& name,
                                                 const FieldGroup::Basis::Type base,
                                                 const std::string& space_lib_name )
{
  return create_space_and_field_group(name,base,space_lib_name,topology());
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup& CMesh::create_space_and_field_group( const std::string& name,
                                                 const FieldGroup::Basis::Type base,
                                                 const std::string& space_lib_name,
                                                 CRegion& topology )
{
  create_space(name,base,space_lib_name);
  return create_field_group(name,base,name,topology);
}

////////////////////////////////////////////////////////////////////////////////

Geometry& CMesh::geometry() const
{
  return *m_nodes;
}

////////////////////////////////////////////////////////////////////////////////

CMeshElements& CMesh::elements() const
{
  return *m_elements;
}

//////////////////////////////////////////////////////////////////////////////

void CMesh::signature_write_mesh ( SignalArgs& node)
{
  SignalOptions options( node );

  options.add_option< OptionT<std::string> >("file" , name() + ".msh" )
      ->description("File to write" );

  boost_foreach (Field& field, find_components_recursively<Field>(*this))
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

  boost_foreach( Field& field, find_components_recursively<Field>(*this))
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
