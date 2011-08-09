// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/assign/list_of.hpp>
#include "Common/CLink.hpp"

#include "Common/FindComponents.hpp"
#include "Common/StringConversion.hpp"
#include "Common/OptionT.hpp"
#include "Common/Signal.hpp"
#include "Common/XML/SignalOptions.hpp"
#include "Common/MPI/PE.hpp"

#include "Mesh/CConnectivity.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CSpace.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CEntities::MeshSpaces::Convert::Convert()
{
  all_fwd = boost::assign::map_list_of
      ( CEntities::MeshSpaces::SPACE0,    "space[0]" )
      ( CEntities::MeshSpaces::MESH_NODES,    "mesh_nodes" );

  all_rev = boost::assign::map_list_of
      ("space[0]",       CEntities::MeshSpaces::SPACE0 )
      ("mesh_nodes",     CEntities::MeshSpaces::MESH_NODES );
}

////////////////////////////////////////////////////////////////////////////////

CEntities::CEntities ( const std::string& name ) :
  Component ( name )
{
  mark_basic();
  properties()["brief"] = std::string("Holds information of elements of one type");
  properties()["description"] = std::string("Container component that stores the element to node connectivity,\n")
  +std::string("a link to node storage, a list of used nodes, and global numbering unique over all processors");

  m_options.add_option(OptionT<std::string>::create("element_type", std::string("")))
      ->description("Element type")
      ->pretty_name("Element type")
      ->attach_trigger(boost::bind(&CEntities::configure_element_type, this));

  m_global_numbering = create_static_component_ptr<CList<Uint> >(Mesh::Tags::global_elem_indices());
  m_global_numbering->add_tag(Mesh::Tags::global_elem_indices());
  m_global_numbering->properties()["brief"] = std::string("The global element indices (inter processor)");

  m_spaces_group = create_static_component_ptr<CGroup>("spaces");
  m_spaces_group->mark_basic();

  m_nodes = create_static_component_ptr<CLink>(Mesh::Tags::nodes());
  m_nodes->add_tag(Mesh::Tags::nodes());

  m_rank = create_static_component_ptr< CList<Uint> >("rank");
  m_rank->add_tag("rank");

  regist_signal ( "create_space" )
      ->connect ( boost::bind ( &CEntities::signal_create_space, this, _1 ) )
      ->description( "Create space for other interpretations of fields (e.g. high order)" )
      ->pretty_name( "Create space" )
      ->signature(boost::bind(&CEntities::signature_create_space, this, _1));

}

////////////////////////////////////////////////////////////////////////////////

CEntities::~CEntities()
{
}

//////////////////////////////////////////////////////////////////////////////

void CEntities::initialize(const std::string& element_type_name)
{
  configure_option("element_type",element_type_name);
  cf_assert(is_not_null(m_element_type));
}

void CEntities::initialize(const std::string& element_type_name, Geometry& nodes)
{
  assign_geometry(nodes);
  initialize(element_type_name);
}

void CEntities::assign_geometry(Geometry& nodes)
{
  m_nodes->link_to(nodes.follow());
}

////////////////////////////////////////////////////////////////////////////////

void CEntities::configure_element_type()
{
  const std::string etype_name = option("element_type").value<std::string>();
  if (is_not_null(m_element_type))
  {
    remove_component(m_element_type->name());
  }
  m_element_type = build_component_abstract_type<ElementType>( etype_name, etype_name );
  m_element_type->rename(m_element_type->derived_type_name());
  add_component( m_element_type );

  if (exists_space(MeshSpaces::SPACE0))
    m_spaces[MeshSpaces::SPACE0]->configure_option("shape_function",m_element_type->shape_function().derived_type_name());
  else
  {
    CSpace& space0 = create_space(MeshSpaces::to_str(MeshSpaces::SPACE0),element_type().shape_function().derived_type_name());
    space0.add_tag(MeshSpaces::to_str(MeshSpaces::SPACE0));

  }

  if ( exists_space(MeshSpaces::MESH_NODES) )
  {
    m_spaces[MeshSpaces::MESH_NODES]->configure_option("shape_function",m_element_type->shape_function().derived_type_name());
  }
  else
  {
    CSpace& mesh_nodes = create_space(MeshSpaces::to_str(MeshSpaces::MESH_NODES),element_type().shape_function().derived_type_name());
    mesh_nodes.add_tag(MeshSpaces::to_str(MeshSpaces::MESH_NODES));
  }
}

//////////////////////////////////////////////////////////////////////////////

ElementType& CEntities::element_type() const
{
  cf_assert_desc("element_type not initialized", is_not_null(m_element_type));
  return *m_element_type;
}

//////////////////////////////////////////////////////////////////////////////

Geometry& CEntities::geometry() const
{
  return m_nodes->follow()->as_type<Geometry>();
}

////////////////////////////////////////////////////////////////////////////////

CList<Uint>& CEntities::used_nodes(Component& parent, const bool rebuild)
{
  CList<Uint>::Ptr used_nodes = find_component_ptr_with_tag<CList<Uint> >(parent,Mesh::Tags::nodes_used());
  if (rebuild && is_not_null(used_nodes))
  {
    parent.remove_component(*used_nodes);
    used_nodes.reset();
  }

  if (is_null(used_nodes))
  {
    used_nodes = parent.create_component_ptr<CList<Uint> >(Mesh::Tags::nodes_used());
    used_nodes->add_tag(Mesh::Tags::nodes_used());
    used_nodes->properties()["brief"] = std::string("The local node indices used by the parent component");

    // Assemble all unique node numbers in a set
    std::set<Uint> node_set;

    if ( CEntities::Ptr elements = parent.as_ptr<CEntities>() )
    {
      const Uint nb_elems = elements->size();
      for (Uint idx=0; idx<nb_elems; ++idx)
      {
        boost_foreach(const Uint node, elements->get_nodes(idx))
        {
          node_set.insert(node);
        }
      }
    }
    else
    {
      boost_foreach(CEntities& elements, find_components_recursively<CEntities>(parent))
      {
        const Uint nb_elems = elements.size();
        for (Uint idx=0; idx<nb_elems; ++idx)
        {
          boost_foreach(const Uint node, elements.get_nodes(idx))
          {
            node_set.insert(node);
          }
        }
      }
    }

    // Copy the set to the node_list

    used_nodes->resize(node_set.size());

    CList<Uint>::ListT& nodes_array = used_nodes->array();
    Uint cnt=0;
    boost_foreach(const Uint node, node_set)
      nodes_array[cnt++] = node;


  }
  return *used_nodes;
}

////////////////////////////////////////////////////////////////////////////////

Uint CEntities::size() const
{
  throw ShouldNotBeHere( FromHere(), " This virtual function has to be overloaded. ");
}

CTable<Uint>::ConstRow CEntities::get_nodes(const Uint elem_idx) const
{
  throw ShouldNotBeHere( FromHere(), " This virtual function has to be overloaded. ");
}

////////////////////////////////////////////////////////////////////////////////

CSpace& CEntities::create_space( const std::string& name, const std::string& shape_function_builder_name )
{
  Uint nb_existing_spaces = m_spaces.size();
  CSpace::Ptr space = m_spaces_group->create_component_ptr<CSpace>(name);
  space->configure_option("shape_function",shape_function_builder_name);
  m_spaces.push_back(space);
  return *space;
}

////////////////////////////////////////////////////////////////////////////////

const CSpace& CEntities::space (const Uint space_idx) const
{
  return *m_spaces[space_idx];
}

////////////////////////////////////////////////////////////////////////////////

CSpace& CEntities::space (const std::string& space_name) const
{
  return m_spaces_group->get_child(space_name).as_type<CSpace>();
}

////////////////////////////////////////////////////////////////////////////////

bool CEntities::exists_space(const Uint space_idx) const
{
  bool exists = false;
  if (m_spaces.size() > space_idx)
    if ( is_not_null (m_spaces[space_idx]) )
      exists = true;
  return exists;
}

////////////////////////////////////////////////////////////////////////////////

bool CEntities::exists_space(const std::string& name) const
{
  return is_not_null(m_spaces_group->get_child_ptr(name));
}

////////////////////////////////////////////////////////////////////////////////

Uint CEntities::space_idx(const std::string& name) const
{
  for (Uint i=0; i<m_spaces.size(); ++i)
  {
    if ( is_not_null(m_spaces[i]) )
      if (m_spaces[i]->name() == name)
        return i;
  }
  throw ValueNotFound(FromHere(), "Space with name ["+name+"] not found for elements ["+uri().path()+"]");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix CEntities::get_coordinates(const Uint elem_idx) const
{
  throw Common::NotImplemented(FromHere(),"Should implement in derived class");
  return RealMatrix(1,1);
}

////////////////////////////////////////////////////////////////////////////////

void CEntities::put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const
{
  throw Common::NotImplemented(FromHere(),"Should implement in derived class");
}

////////////////////////////////////////////////////////////////////////////////

void CEntities::allocate_coordinates(RealMatrix& coords) const
{
  coords.resize(element_type().nb_nodes(),element_type().dimension());
}

////////////////////////////////////////////////////////////////////////////////

void CEntities::signature_create_space ( SignalArgs& node)
{
  XML::SignalOptions options( node );
  options.add_option< OptionT<std::string> >("name" , std::string("space["+to_str(m_spaces.size())+"]") )
      ->description("Name to add to space");

  options.add_option< OptionT<std::string> >("shape_function" , std::string("CF.Mesh.SF.SFLineP0Lagrange") )
      ->description("Shape Function to add as space");
}

////////////////////////////////////////////////////////////////////////////////

void CEntities::signal_create_space ( SignalArgs& node )
{
  XML::SignalOptions options( node );

  std::string name = "space["+to_str(m_spaces.size())+"]";
  if (options.check("name"))
    name = options.value<std::string>("name");

  std::string shape_function_builder = options.value<std::string>("shape_function");

  CSpace& space = create_space(name, shape_function_builder);
}

////////////////////////////////////////////////////////////////////////////////

bool CEntities::is_ghost(const Uint idx) const
{
  cf_assert_desc(to_str(idx)+">="+to_str(size()),idx < size());
  cf_assert(size() == m_rank->size());
  cf_assert(idx<m_rank->size());
  return (*m_rank)[idx] != MPI::PE::instance().rank();
}

////////////////////////////////////////////////////////////////////////////////

bool IsElementsVolume::operator()(const CEntities::Ptr& component)
{
  return component->element_type().dimension() == component->element_type().dimensionality();
}

bool IsElementsVolume::operator()(const CEntities& component)
{
  return component.element_type().dimension() == component.element_type().dimensionality();
}

bool IsElementsSurface::operator()(const CEntities::Ptr& component)
{
  return component->element_type().dimension() == component->element_type().dimensionality() + 1;
}

bool IsElementsSurface::operator()(const CEntities& component)
{
  return component.element_type().dimension() == component.element_type().dimensionality() + 1;
}

////////////////////////////////////////////////////////////////////////////////
} // Mesh
} // CF
