// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/functional/hash.hpp>

#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CLink.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Core.hpp"
#include "Common/EventHandler.hpp"
#include "Common/StringConversion.hpp"
#include "Common/CLink.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Common/MPI/PE.hpp"

#include "Math/VariablesDescriptor.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/FieldGroup.hpp"
#include "Mesh/Field.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CUnifiedData.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CFaces.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CConnectivity.hpp"


namespace CF {
namespace Mesh {

using namespace boost::assign;

using namespace Common;
using namespace Common::Comm;

Common::ComponentBuilder < FieldGroup, Component, LibMesh >  FieldGroup_Builder;

////////////////////////////////////////////////////////////////////////////////

FieldGroup::Basis::Convert::Convert()
{
  all_fwd = boost::assign::map_list_of
      ( FieldGroup::Basis::POINT_BASED, "point_based" )
      ( FieldGroup::Basis::ELEMENT_BASED, "element_based" )
      ( FieldGroup::Basis::CELL_BASED, "cell_based" )
      ( FieldGroup::Basis::FACE_BASED, "face_based" );

  all_rev = boost::assign::map_list_of
      ("point_based",    FieldGroup::Basis::POINT_BASED )
      ("element_based",  FieldGroup::Basis::ELEMENT_BASED )
      ("cell_based",     FieldGroup::Basis::CELL_BASED )
      ("face_based",     FieldGroup::Basis::FACE_BASED );
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup::Basis::Convert& FieldGroup::Basis::Convert::instance()
{
  static FieldGroup::Basis::Convert instance;
  return instance;
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup::FieldGroup ( const std::string& name  ) :
  Component( name ),
  m_basis(Basis::INVALID),
  m_space("invalid"),
  m_size(0u)
{
  mark_basic();

  // Option "topology"
  m_options.add_option< OptionURI >("topology",URI("cpath:"))
      ->description("The region these fields apply to")
      ->attach_trigger( boost::bind( &FieldGroup::config_topology, this) )
      ->mark_basic();

  // Option "type"
  m_options.add_option< OptionT<std::string> >("type", Basis::to_str(m_basis))
      ->description("The type of the field")
      ->attach_trigger ( boost::bind ( &FieldGroup::config_type,   this ) )
      ->mark_basic();
  option("type").restricted_list() =  list_of
      (Basis::to_str(Basis::POINT_BASED))
      (Basis::to_str(Basis::ELEMENT_BASED))
      (Basis::to_str(Basis::CELL_BASED))
      (Basis::to_str(Basis::FACE_BASED));

  // Option "space
  m_options.add_option< OptionT<std::string> >("space", m_space)
    ->description("The space of the field is based on")
    ->attach_trigger ( boost::bind ( &FieldGroup::config_space,   this ) )
    ->mark_basic();

  // Static components
  m_topology = create_static_component_ptr<CLink>("topology");
  m_elements_lookup = create_static_component_ptr<CUnifiedData>("elements_lookup");

  m_rank = create_static_component_ptr< CList<Uint> >("rank");
  m_rank->add_tag("rank");

  m_glb_idx = create_static_component_ptr< CList<Uint> >(Mesh::Tags::global_indices());
  m_glb_idx->add_tag(Mesh::Tags::global_indices());


  // Event handlers
  Core::instance().event_handler().connect_to_event("mesh_loaded", this, &FieldGroup::on_mesh_changed_event);
  Core::instance().event_handler().connect_to_event("mesh_changed", this, &FieldGroup::on_mesh_changed_event);

}

////////////////////////////////////////////////////////////////////////////////

void FieldGroup::config_topology()
{
  URI topology_uri;
  option("topology").put_value(topology_uri);
  CRegion::Ptr topology = access_component(topology_uri).as_ptr<CRegion>();
  if ( is_null(topology) )
    throw CastingFailed (FromHere(), "Topology must be of a CRegion or derived type");
  m_topology->link_to(topology);

  if (m_basis != Basis::INVALID && m_space != "invalid")
    update();
}

////////////////////////////////////////////////////////////////////////////////

void FieldGroup::config_type()
{
  m_basis = Basis::to_enum( option("type").value<std::string>() );

  if (m_topology->is_linked() && m_space != "invalid")
    update();
}

////////////////////////////////////////////////////////////////////////////////


void FieldGroup::config_space()
{
  m_space = option("space").value<std::string>();

  if (m_topology->is_linked() && m_basis != Basis::INVALID)
    update();
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup::~FieldGroup()
{
}


////////////////////////////////////////////////////////////////////////////////

void FieldGroup::resize(const Uint size)
{
  m_size = size;
  m_glb_idx->resize(m_size);
  m_rank->resize(m_size);
  properties()["size"]=m_size;

  boost_foreach(Field& field, find_components<Field>(*this))
    field.resize(m_size);
}

//////////////////////////////////////////////////////////////////////////////

bool FieldGroup::is_ghost(const Uint idx) const
{
  cf_assert_desc(to_str(idx)+">="+to_str(size()),idx < size());
  cf_assert(size() == m_rank->size());
  cf_assert(idx<m_rank->size());
  return (*m_rank)[idx] != PE::instance().rank();
}

////////////////////////////////////////////////////////////////////////////////

CRegion& FieldGroup::topology() const
{
  return *m_topology->follow()->as_ptr<CRegion>();
}

////////////////////////////////////////////////////////////////////////////////

Field& FieldGroup::create_field(const std::string &name, const std::string& variables_description)
{

  Field& field = create_component<Field>(name);
  field.set_field_group(*this);
  field.set_topology(topology());
  field.set_basis(m_basis);

  if (variables_description == "scalar_same_name")
    field.create_descriptor(name+"[scalar]",parent().as_type<CMesh>().dimension());
  else
    field.create_descriptor(variables_description,parent().as_type<CMesh>().dimension());

  field.resize(m_size);
  return field;

//  std::vector<std::string> names;
//  std::vector<std::string> types;
//  BOOST_FOREACH(std::string var, tokenized_variables)
//  {
//    boost::regex e_variable("([[:word:]]+)?[[:space:]]*\\[[[:space:]]*([[:word:]]+)[[:space:]]*\\]");

//    boost::match_results<std::string::const_iterator> what;
//    if (regex_search(var,what,e_variable))
//    {
//      names.push_back(what[1]);
//      types.push_back(what[2]);
//    }
//    else
//      throw ShouldNotBeHere(FromHere(), "No match found for VarType " + var);
//  }

//  Field& field = create_component<Field>(name);
//  field.set_field_group(*this);
//  field.set_topology(topology());
//  field.set_basis(m_basis);
//  field.configure_option("var_names",names);
//  field.configure_option("var_types",types);
//  field.resize(m_size);
//  return field;
}

////////////////////////////////////////////////////////////////////////////////

Field& FieldGroup::create_field(const std::string &name, Math::VariablesDescriptor& variables_descriptor)
{
  Field& field = create_component<Field>(name);
  field.set_field_group(*this);
  field.set_topology(topology());
  field.set_basis(m_basis);
  field.set_descriptor(variables_descriptor);
  field.descriptor().configure_option("dimension",parent().as_type<CMesh>().dimension());
  field.resize(m_size);
  return field;
}

////////////////////////////////////////////////////////////////////////////////

void FieldGroup::check_sanity()
{
  boost_foreach(Field& field, find_components<Field>(*this))
  {
    if (field.size() != m_size)
      throw InvalidStructure(FromHere(),"field ["+field.uri().string()+"] has a size "+to_str(field.size())+" != supposed "+to_str(m_size));
  }

  boost_foreach(CList<Uint>& list, find_components<CList<Uint> >(*this))
  {
    if (list.size() != m_size)
      throw InvalidStructure(FromHere(),"list ["+list.uri().string()+"] has a size "+to_str(list.size())+" != supposed "+to_str(m_size));
  }
}

////////////////////////////////////////////////////////////////////////////////

boost::iterator_range< Common::ComponentIterator<CEntities> > FieldGroup::entities_range()
{
  std::vector<CEntities::Ptr> elements_vec(elements_lookup().components().size());
  for (Uint c=0; c<elements_vec.size(); ++c)
    elements_vec[c] = elements_lookup().components()[c]->as_ptr<CEntities>();

  ComponentIterator<CEntities> begin_iter(elements_vec,0);
  ComponentIterator<CEntities> end_iter(elements_vec,elements_vec.size());
  return boost::make_iterator_range(begin_iter,end_iter);
}

////////////////////////////////////////////////////////////////////////////////

boost::iterator_range< Common::ComponentIterator<CElements> > FieldGroup::elements_range()
{
  std::vector<CElements::Ptr> elements_vec(elements_lookup().components().size());
  for (Uint c=0; c<elements_vec.size(); ++c)
    elements_vec[c] = elements_lookup().components()[c]->as_ptr<CElements>();

  ComponentIterator<CElements> begin_iter(elements_vec,0);
  ComponentIterator<CElements> end_iter(elements_vec,elements_vec.size());
  return boost::make_iterator_range(begin_iter,end_iter);
}

////////////////////////////////////////////////////////////////////////////////

Common::ComponentIteratorRange<Field> FieldGroup::fields()
{
  return find_components<Field>(*this);
}

////////////////////////////////////////////////////////////////////////////////

Field& FieldGroup::field(const std::string& name) const
{
  return get_child(name).as_type<Field>();
}

////////////////////////////////////////////////////////////////////////////////

void FieldGroup::on_mesh_changed_event( SignalArgs& args )
{
  Common::XML::SignalOptions options( args );

  URI mesh_uri = options.value<URI>("mesh_uri");
  if (mesh_uri.is_relative())
  {
    throw InvalidURI(FromHere(),"URI "+to_str(mesh_uri)+" should be absolute");
  }
  CMesh& mesh_arg = access_component(mesh_uri).as_type<CMesh>();

  CMesh& this_mesh = find_parent_component<CMesh>(*this);

  if (&this_mesh == &mesh_arg)
  {
    if (m_topology->is_linked() == false)
      throw SetupError(FromHere(), "topology of field_group ["+uri().string()+"] not configured");
    if (m_basis == Basis::INVALID)
      throw SetupError(FromHere(), "type of field_group ["+uri().string()+"] not configured");
    if (m_space == "invalid")
      throw SetupError(FromHere(), "space of field_group ["+uri().string()+"] not configured");

    update();
  }
}

////////////////////////////////////////////////////////////////////////////////

void FieldGroup::update()
{
  elements_lookup().reset();

  switch (m_basis)
  {
    case Basis::POINT_BASED:
    case Basis::ELEMENT_BASED:
      boost_foreach(CEntities& entities, find_components_recursively<CEntities>(topology()))
        elements_lookup().add(entities);
      break;
    case Basis::CELL_BASED:
      boost_foreach(CCells& cells, find_components_recursively<CCells>(topology()))
        elements_lookup().add(cells);
      break;
    case Basis::FACE_BASED:
      boost_foreach(CFaces& faces, find_components_recursively<CFaces>(topology()))
        elements_lookup().add(faces);
      break;
    default:
      throw InvalidStructure(FromHere(), "basis not set");
  }

  bind_space();
//  if (m_basis != Basis::POINT_BASED)
//  {
//    Uint new_size = 0;
//    boost_foreach(CEntities& entities, entities_range())
//      new_size += entities.space(m_space).nb_states() * entities.size();
//    resize(new_size);
//    bind_space();
//  }
//  else
//  {
//    if (m_space != CEntities::MeshSpaces::to_str(CEntities::MeshSpaces::MESH_NODES))
//    {
//      bind_space();
//    }
//  }

  check_sanity();
}

////////////////////////////////////////////////////////////////////////////////

void FieldGroup::bind_space()
{
  if (m_topology->is_linked() == false)
    throw SetupError(FromHere(), "topology of field_group ["+uri().string()+"] not configured");
  if (m_space == "invalid")
    throw SetupError(FromHere(), "space of field_group ["+uri().string()+"] not configured");
  if (m_basis == Basis::INVALID)
    throw SetupError(FromHere(), "type of field_group ["+uri().string()+"] not configured");

  if (m_space != CEntities::MeshSpaces::to_str(CEntities::MeshSpaces::MESH_NODES))
    create_connectivity_in_space();
  // else the connectivity must be manually created by mesh reader or mesh transformer

  boost_foreach(CEntities& entities, entities_range())
    entities.space(m_space).get_child("bound_fields").as_type<CLink>().link_to(*this);
}

std::size_t hash_value(const RealMatrix& coords)
{
  std::size_t seed=0;
  for (Uint i=0; i<coords.rows(); ++i)
  for (Uint j=0; j<coords.cols(); ++j)
  {
    // multiply with 1e-5 (arbitrary) to avoid hash collisions
    boost::hash_combine(seed,1e-3*coords(i,j));
  }
  return seed;
}

void FieldGroup::create_connectivity_in_space()
{
  if (m_topology->is_linked() == false)
    throw SetupError(FromHere(), "topology of field_group ["+uri().string()+"] not configured");
  if (m_space == "invalid")
    throw SetupError(FromHere(), "space of field_group ["+uri().string()+"] not configured");
  if (m_basis == Basis::INVALID)
    throw SetupError(FromHere(), "type of field_group ["+uri().string()+"] not configured");


  if (m_basis == Basis::POINT_BASED)
  {
    std::set<std::size_t> points;
    RealMatrix elem_coordinates;
    Uint dim = DIM_0D;

    // step 1: collect nodes in a set
    // ------------------------------
    boost_foreach(CEntities& entities, elements_range())
    {
      const ShapeFunction& shape_function = entities.space(m_space).shape_function();
      for (Uint elem=0; elem<entities.size(); ++elem)
      {
        elem_coordinates = entities.get_coordinates(elem);
        for (Uint node=0; node<shape_function.nb_nodes(); ++node)
        {
          RealVector space_coordinates = entities.element_type().shape_function().value(shape_function.local_coordinates().row(node)) * elem_coordinates ;
          std::size_t hash = hash_value(space_coordinates);
          points.insert( hash );
        }
      }
    }

    Field& coordinates = create_field("coordinates","coords[vector]");
    coordinates.resize(points.size());
    m_coordinates = coordinates.as_ptr<Field>();

    // step 2: collect nodes in a set
    // ------------------------------
    boost_foreach(CEntities& entities, entities_range())
    {
      entities.space(m_space).get_child("bound_fields").as_type<CLink>().link_to(*this);
      const ShapeFunction& shape_function = entities.space(m_space).shape_function();
      CConnectivity& connectivity = entities.space(m_space).connectivity();
      connectivity.set_row_size(shape_function.nb_nodes());
      connectivity.resize(entities.size());
      for (Uint elem=0; elem<entities.size(); ++elem)
      {
        elem_coordinates = entities.get_coordinates(elem);
        for (Uint node=0; node<shape_function.nb_nodes(); ++node)
        {
          RealVector space_coordinates = entities.element_type().shape_function().value(shape_function.local_coordinates().row(node)) * elem_coordinates ;
          std::size_t hash = hash_value(space_coordinates);
          Uint idx = std::distance(points.begin(), points.find(hash));
          connectivity[elem][node] = idx;
          coordinates.set_row(idx, space_coordinates);
        }
      }
    }

    // step 3: resize
    // --------------
    resize(points.size());

    // step 4: add lookup to connectivity tables
    // -----------------------------------------
    boost_foreach(CEntities& entities, entities_range())
        entities.space(m_space).connectivity().create_lookup().add(*this);
  }
  else // If Element-based
  {

    // Check if this space is not already bound to another field_group
    boost_foreach(CEntities& entities, entities_range())
    {
      if (entities.space(m_space).is_bound_to_fields() > 0)
        throw SetupError(FromHere(), "Space ["+entities.space(m_space).uri().string()+"] is already bound to\n"
                         "fields ["+entities.space(m_space).bound_fields().uri().string()+"]\nCreate a new space for field_group ["+uri().string()+"]");
    }

    // Assign the space connectivity table
    Uint field_idx = 0;
    boost_foreach(CEntities& entities, entities_range())
    {
      CSpace& space = entities.space(m_space);
      space.get_child("bound_fields").as_type<CLink>().link_to(*this);
      space.make_proxy(field_idx);
      field_idx += entities.size()*space.nb_states();
    }
    resize(field_idx);
    boost_foreach(CEntities& entities, entities_range())
        entities.space(m_space).connectivity().create_lookup().add(*this);
  }
}

////////////////////////////////////////////////////////////////////////////////

CTable<Uint>::ConstRow FieldGroup::indexes_for_element(const CEntities& elements, const Uint idx) const
{
  CSpace& space = elements.space(m_space);
  cf_assert_desc("space not bound to this field_group", &space.bound_fields() == this);
  return space.indexes_for_element(idx);
}

////////////////////////////////////////////////////////////////////////////////

CTable<Uint>::ConstRow FieldGroup::indexes_for_element(const Uint unified_idx) const
{
  Component::Ptr component;
  Uint elem_idx;
  boost::tie(component,elem_idx) = elements_lookup().location(unified_idx);
  return indexes_for_element(component->as_type<CEntities>(),elem_idx);
}

////////////////////////////////////////////////////////////////////////////////

Field& FieldGroup::coordinates() const
{
  if (is_null(m_coordinates))
    throw ValueNotFound(FromHere(),"FieldGroup ["+uri().string()+"] has no coordinates field");

  return *m_coordinates;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
