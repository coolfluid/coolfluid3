// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>

#include "common/Builder.hpp"
#include "common/Link.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionT.hpp"
#include "common/StringConversion.hpp"
#include "common/Signal.hpp"
#include "common/Tags.hpp"

#include "common/PE/Comm.hpp"

#include "common/XML/SignalOptions.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/LibMesh.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/CRegion.hpp"
#include "mesh/Geometry.hpp"
#include "mesh/FieldGroup.hpp"
#include "mesh/MeshElements.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/WriteMesh.hpp"
#include "mesh/MeshMetadata.hpp"
#include "mesh/CCells.hpp"
#include "mesh/CFaces.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::XML;
using namespace common::PE;

common::ComponentBuilder < Mesh, Component, LibMesh > Mesh_Builder;

////////////////////////////////////////////////////////////////////////////////

Mesh::Mesh ( const std::string& name  ) :
  Component ( name ),
  m_dimension(0u),
  m_dimensionality(0u)
{
  mark_basic(); // by default meshes are visible

  m_properties.add_property("nb_cells",Uint(0));
  m_properties.add_property("nb_faces",Uint(0));
  m_properties.add_property("nb_nodes",Uint(0));
  m_properties.add_property("dimensionality",Uint(0));
  m_properties.add_property(common::Tags::dimension(),Uint(0));

  m_elements   = create_static_component_ptr<MeshElements>("elements");
  m_topology   = create_static_component_ptr<CRegion>("topology");
  m_metadata   = create_static_component_ptr<MeshMetadata>("metadata");

  regist_signal ( "write_mesh" )
      ->description( "Write mesh, guessing automatically the format" )
      ->pretty_name("Write Mesh" )
      ->connect   ( boost::bind ( &Mesh::signal_write_mesh,    this, _1 ) )
      ->signature ( boost::bind ( &Mesh::signature_write_mesh, this, _1 ) );

  m_nodes = create_static_component_ptr<Geometry>(mesh::Tags::nodes());
  m_nodes->add_tag(mesh::Tags::nodes());

}

////////////////////////////////////////////////////////////////////////////////

Mesh::~Mesh()
{
}

////////////////////////////////////////////////////////////////////////////////

void Mesh::initialize_nodes(const Uint nb_nodes, const Uint dimension)
{
  cf3_assert(dimension > 0);

  geometry().configure_option("type",    FieldGroup::Basis::to_str(FieldGroup::Basis::POINT_BASED));
  geometry().configure_option("space",   std::string(Tags::geometry()));
  geometry().configure_option("topology",topology().uri());

  geometry().coordinates().set_field_group(geometry());
  geometry().coordinates().set_topology(geometry().topology());
  geometry().coordinates().set_basis(FieldGroup::Basis::POINT_BASED);
  geometry().coordinates().descriptor().configure_option(common::Tags::dimension(),dimension);
  geometry().resize(nb_nodes);

  cf3_assert(geometry().size() == nb_nodes);
  cf3_assert(geometry().coordinates().row_size() == dimension);
  m_dimension = dimension;
  property(common::Tags::dimension()) = m_dimension;
  property("nb_nodes")  = geometry().size();
}

////////////////////////////////////////////////////////////////////////////////

void Mesh::update_statistics()
{
  cf3_assert(m_dimension == geometry().coordinates().row_size() );
  boost_foreach ( CEntities& elements, find_components_recursively<CEntities>(topology()) )
    m_dimensionality = std::max(m_dimensionality,elements.element_type().dimensionality());

  Uint nb_cells = 0;
  boost_foreach ( CCells& elements, find_components_recursively<CCells>(topology()) )
    nb_cells += elements.size();

  Uint nb_faces = 0;
  boost_foreach ( CFaces& elements, find_components_recursively<CFaces>(topology()) )
    nb_faces += elements.size();

  property(common::Tags::dimension()) = m_dimension;
  property("dimensionality")= m_dimensionality;
  property("nb_cells") = nb_cells;
  property("nb_faces") = nb_faces;
  property("nb_nodes") = geometry().size();
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup& Mesh::create_field_group( const std::string& name,
                                       const FieldGroup::Basis::Type base )
{
  return create_field_group ( name, base, name, topology() );
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup& Mesh::create_field_group( const std::string& name,
                                       const FieldGroup::Basis::Type base,
                                       const std::string& space )
{
  return create_field_group ( name, base, space, topology() );
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup& Mesh::create_field_group( const std::string& name,
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

void Mesh::create_space( const std::string& name, const FieldGroup::Basis::Type base, const std::string& space_lib_name)
{
  create_space(name,base,space_lib_name,topology());
}

////////////////////////////////////////////////////////////////////////////////

void Mesh::create_space( const std::string& name, const FieldGroup::Basis::Type base, const std::string& space_lib_name, CRegion& topology)
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
    boost_foreach(CEntities& elements, find_components_recursively_with_tag<CEntities>(topology,mesh::Tags::face_entity()))
      elements.create_space(name,space_lib_name+"."+elements.element_type().shape_name());
    break;
  case FieldGroup::Basis::INVALID:
  default:
    throw BadValue(FromHere(),"value "+FieldGroup::Basis::to_str(base)+" not supported for base");
  }
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup& Mesh::create_space_and_field_group( const std::string& name,
                                                 const FieldGroup::Basis::Type base,
                                                 const std::string& space_lib_name )
{
  return create_space_and_field_group(name,base,space_lib_name,topology());
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup& Mesh::create_space_and_field_group( const std::string& name,
                                                 const FieldGroup::Basis::Type base,
                                                 const std::string& space_lib_name,
                                                 CRegion& topology )
{
  create_space(name,base,space_lib_name);
  return create_field_group(name,base,name,topology);
}

////////////////////////////////////////////////////////////////////////////////

Geometry& Mesh::geometry() const
{
  return *m_nodes;
}

////////////////////////////////////////////////////////////////////////////////

MeshElements& Mesh::elements() const
{
  return *m_elements;
}

//////////////////////////////////////////////////////////////////////////////

void Mesh::signature_write_mesh ( SignalArgs& node)
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

void Mesh::signal_write_mesh ( SignalArgs& node )
{
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

  write_mesh(fpath,fields);
}

void Mesh::write_mesh( const URI& file, const std::vector<URI> fields)
{
  WriteMesh& mesh_writer = create_component<WriteMesh>("writer");
  mesh_writer.write_mesh(*this,file,fields);
  remove_component(mesh_writer);
}

////////////////////////////////////////////////////////////////////////////////

void Mesh::check_sanity() const
{
  std::stringstream message;

  if (dimension() == 0)
    message << "- mesh.dimension not configured" << std::endl;

  if (dimensionality() == 0)
    message << "- mesh.dimensionality not configured" << std::endl;

  if (dimensionality() > dimension())
    message << "- dimensionality ["<< dimensionality()<<"]  >  dimension ["<<dimension()<<"]" << std::endl;

  if(geometry().coordinates().row_size() != dimension())
    message << "- coordinates dimension does not match mesh.dimension" << std::endl;

  boost_foreach(const FieldGroup& field_group, find_components_recursively<FieldGroup>(*this))
  {
    if (field_group.rank().size() != field_group.size())
      message << "- " << field_group.uri().string() << ": size() ["<<field_group.size()<<"] != rank().size() ["<<field_group.rank().size()<<"]"<<std::endl;

    if (field_group.glb_idx().size() != field_group.size())
      message << "- " << field_group.uri().string() << ": size() ["<<field_group.size()<<"] != glb_idx().size() ["<<field_group.glb_idx().size()<<"]"<<std::endl;

    boost_foreach(const Field& field, find_components_recursively<Field>(field_group))
    {
      if (field.size() != field_group.size())
        message << "- " << field.uri().string() << ": size() ["<<field.size()<<"] != field_group.size() ["<<field_group.size()<<"]"<<std::endl;
    }
  }

  if (Comm::instance().is_active())
  {
    std::set<Uint> unique_node_gids;
    boost_foreach(const Uint gid, geometry().glb_idx().array())
    {
      std::pair<std::set<Uint>::iterator, bool > inserted = unique_node_gids.insert(gid);
      if (inserted.second == false)
      {
        message << "- " << geometry().glb_idx().uri().string() << " has non-unique entries.  (entry "<<gid<<" exists more than once, no further checks)" << std::endl;
        break;
      }
    }
  }

  std::set<Uint> unique_elem_gids;
  boost_foreach(const CEntities& entities, find_components_recursively<CEntities>(*this))
  {
    if (entities.rank().size() != entities.size())
      message << "- " << entities.uri().string() << ": size() ["<<entities.size()<<"] != rank().size() ["<<entities.rank().size()<<"]"<<std::endl;
    if (entities.glb_idx().size() != entities.size())
      message << "- " << entities.uri().string() << ": size() ["<<entities.size()<<"] != glb_idx().size() ["<<entities.glb_idx().size()<<"]"<<std::endl;

    if (Comm::instance().is_active())
    {
      boost_foreach(const Uint gid, entities.glb_idx().array())
      {
        std::pair<std::set<Uint>::iterator, bool > inserted = unique_elem_gids.insert(gid);
        if (inserted.second == false)
        {
          message << "- " << entities.glb_idx().uri().string() << " has non-unique entries.  (entry "<<gid<<" exists more than once, no further checks)" << std::endl;
          break;
        }
      }
    }

  }

  std::string message_str = message.str();
  if (!message_str.empty())
    throw InvalidStructure(FromHere(), "Mesh "+uri().string()+" is not sane:\n"+message_str);
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
