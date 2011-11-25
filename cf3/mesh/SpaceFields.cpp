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

#include "common/Log.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/Builder.hpp"
#include "common/Link.hpp"
#include "common/FindComponents.hpp"
#include "common/Core.hpp"
#include "common/EventHandler.hpp"
#include "common/StringConversion.hpp"
#include "common/Link.hpp"
#include "common/Tags.hpp"
#include "common/DynTable.hpp"
#include "common/XML/SignalOptions.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/CommPattern.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/LibMesh.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/Field.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/Region.hpp"
#include "mesh/Mesh.hpp"
#include "common/List.hpp"
#include "mesh/UnifiedData.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

#include "math/Consts.hpp"
#define UNKNOWN math::Consts::uint_max()

namespace cf3 {
namespace mesh {

using namespace boost::assign;

using namespace common;
using namespace common::PE;
using namespace common::XML;

common::ComponentBuilder < SpaceFields, Component, LibMesh >  SpaceFields_Builder;

////////////////////////////////////////////////////////////////////////////////

SpaceFields::Basis::Convert::Convert()
{
  all_fwd = boost::assign::map_list_of
      ( SpaceFields::Basis::POINT_BASED, "point_based" )
      ( SpaceFields::Basis::ELEMENT_BASED, "element_based" )
      ( SpaceFields::Basis::CELL_BASED, "cell_based" )
      ( SpaceFields::Basis::FACE_BASED, "face_based" );

  all_rev = boost::assign::map_list_of
      ("point_based",    SpaceFields::Basis::POINT_BASED )
      ("element_based",  SpaceFields::Basis::ELEMENT_BASED )
      ("cell_based",     SpaceFields::Basis::CELL_BASED )
      ("face_based",     SpaceFields::Basis::FACE_BASED );
}

////////////////////////////////////////////////////////////////////////////////

SpaceFields::Basis::Convert& SpaceFields::Basis::Convert::instance()
{
  static SpaceFields::Basis::Convert instance;
  return instance;
}

////////////////////////////////////////////////////////////////////////////////

SpaceFields::SpaceFields ( const std::string& name  ) :
  Component( name ),
  m_basis(Basis::INVALID),
  m_space("invalid"),
  m_size(0u)
{
  mark_basic();

  // Option "topology"
  options().add_option< OptionURI >("topology",URI("cpath:"))
      ->description("The region these fields apply to")
      ->attach_trigger( boost::bind( &SpaceFields::config_topology, this) )
      ->mark_basic();

  // Option "type"
  options().add_option< OptionT<std::string> >("type", Basis::to_str(m_basis))
      ->description("The type of the field")
      ->attach_trigger ( boost::bind ( &SpaceFields::config_type,   this ) )
      ->mark_basic();
  option("type").restricted_list() =  list_of
      (Basis::to_str(Basis::POINT_BASED))
      (Basis::to_str(Basis::ELEMENT_BASED))
      (Basis::to_str(Basis::CELL_BASED))
      (Basis::to_str(Basis::FACE_BASED));

  // Option "space
  options().add_option< OptionT<std::string> >("space", m_space)
    ->description("The space of the field is based on")
    ->attach_trigger ( boost::bind ( &SpaceFields::config_space,   this ) )
    ->mark_basic();

  // Static components
  m_topology = create_static_component_ptr<Link>("topology");
  m_elements_lookup = create_static_component_ptr<UnifiedData>("elements_lookup");

  m_rank = create_static_component_ptr< common::List<Uint> >("rank");
  m_rank->add_tag("rank");

  m_glb_idx = create_static_component_ptr< common::List<Uint> >(mesh::Tags::global_indices());
  m_glb_idx->add_tag(mesh::Tags::global_indices());


  // Event handlers
  Core::instance().event_handler().connect_to_event("mesh_loaded", this, &SpaceFields::on_mesh_changed_event);
  Core::instance().event_handler().connect_to_event("mesh_changed", this, &SpaceFields::on_mesh_changed_event);

  // Signals
  regist_signal ( "create_field" )
      ->description( "Create Field" )
      ->pretty_name("Create Field" )
      ->connect   ( boost::bind ( &SpaceFields::signal_create_field,    this, _1 ) )
      ->signature ( boost::bind ( &SpaceFields::signature_create_field, this, _1 ) );


}

////////////////////////////////////////////////////////////////////////////////

void SpaceFields::config_topology()
{
  URI topology_uri;
  option("topology").put_value(topology_uri);
  Region::Ptr topology = access_component(topology_uri).as_ptr<Region>();
  if ( is_null(topology) )
    throw CastingFailed (FromHere(), "Topology must be of a Region or derived type");
  m_topology->link_to(topology);

  if (m_basis != Basis::INVALID && m_space != "invalid")
    update();
}

////////////////////////////////////////////////////////////////////////////////

void SpaceFields::config_type()
{
  m_basis = Basis::to_enum( option("type").value<std::string>() );

  if (m_topology->is_linked() && m_space != "invalid")
    update();
}

////////////////////////////////////////////////////////////////////////////////


void SpaceFields::config_space()
{
  m_space = option("space").value<std::string>();

  if (m_topology->is_linked() && m_basis != Basis::INVALID)
    update();
}

////////////////////////////////////////////////////////////////////////////////

SpaceFields::~SpaceFields()
{
}


////////////////////////////////////////////////////////////////////////////////

void SpaceFields::resize(const Uint size)
{
  m_size = size;
  m_glb_idx->resize(m_size);
  m_rank->resize(m_size);
  properties()["size"]=m_size;

  boost_foreach(Field& field, find_components<Field>(*this))
    field.resize(m_size);
}

//////////////////////////////////////////////////////////////////////////////

CommPattern& SpaceFields::comm_pattern()
{
  if(m_comm_pattern.expired())
  {
    PE::CommPattern& comm_pattern = create_component<PE::CommPattern>("CommPattern");
    comm_pattern.insert("gid",glb_idx().array(),false);
    comm_pattern.setup(comm_pattern.get_child("gid").as_ptr<PE::CommWrapper>(),rank().array());
    m_comm_pattern = comm_pattern.as_ptr<common::PE::CommPattern>();
  }

  return *m_comm_pattern.lock();
}

//////////////////////////////////////////////////////////////////////////////

bool SpaceFields::is_ghost(const Uint idx) const
{
  cf3_assert_desc(to_str(idx)+">="+to_str(size()),idx < size());
  cf3_assert(size() == m_rank->size());
  cf3_assert(idx<m_rank->size());
  return (*m_rank)[idx] != Comm::instance().rank();
}

////////////////////////////////////////////////////////////////////////////////

Region& SpaceFields::topology() const
{
  return *m_topology->follow()->as_ptr<Region>();
}

////////////////////////////////////////////////////////////////////////////////

Space& SpaceFields::space(const Entities& entities) const
{
  return entities.space(m_space);
}

////////////////////////////////////////////////////////////////////////////////

Field& SpaceFields::create_field(const std::string &name, const std::string& variables_description)
{

  Field& field = create_component<Field>(name);
  field.set_field_group(*this);
  field.set_topology(topology());
  field.set_basis(m_basis);

  if (variables_description == "scalar_same_name")
    field.create_descriptor(name+"[scalar]",parent().as_type<Mesh>().dimension());
  else
    field.create_descriptor(variables_description,parent().as_type<Mesh>().dimension());

  field.resize(m_size);
  return field;
}

////////////////////////////////////////////////////////////////////////////////

Field& SpaceFields::create_field(const std::string &name, math::VariablesDescriptor& variables_descriptor)
{
  Field& field = create_component<Field>(name);
  field.set_field_group(*this);
  field.set_topology(topology());
  field.set_basis(m_basis);
  field.set_descriptor(variables_descriptor);
  if (variables_descriptor.option(common::Tags::dimension()).value<Uint>() == 0)
    field.descriptor().configure_option(common::Tags::dimension(),parent().as_type<Mesh>().dimension());
  field.resize(m_size);
  return field;
}

////////////////////////////////////////////////////////////////////////////////

void SpaceFields::check_sanity()
{
  boost_foreach(Field& field, find_components<Field>(*this))
  {
    if (field.size() != m_size)
      throw InvalidStructure(FromHere(),"field ["+field.uri().string()+"] has a size "+to_str(field.size())+" != supposed "+to_str(m_size));
  }

  boost_foreach(common::List<Uint>& list, find_components<common::List<Uint> >(*this))
  {
    if (list.size() != m_size)
      throw InvalidStructure(FromHere(),"list ["+list.uri().string()+"] has a size "+to_str(list.size())+" != supposed "+to_str(m_size));
  }
}

////////////////////////////////////////////////////////////////////////////////

boost::iterator_range< common::ComponentIterator<Entities> > SpaceFields::entities_range()
{
  std::vector<Entities::Ptr> elements_vec(elements_lookup().components().size());
  for (Uint c=0; c<elements_vec.size(); ++c)
    elements_vec[c] = elements_lookup().components()[c].lock()->as_ptr<Entities>();

  ComponentIterator<Entities> begin_iter(elements_vec,0);
  ComponentIterator<Entities> end_iter(elements_vec,elements_vec.size());
  return boost::make_iterator_range(begin_iter,end_iter);
}

////////////////////////////////////////////////////////////////////////////////

boost::iterator_range< common::ComponentIterator<Elements> > SpaceFields::elements_range()
{
  std::vector<Elements::Ptr> elements_vec(elements_lookup().components().size());
  for (Uint c=0; c<elements_vec.size(); ++c)
    elements_vec[c] = elements_lookup().components()[c].lock()->as_ptr<Elements>();

  ComponentIterator<Elements> begin_iter(elements_vec,0);
  ComponentIterator<Elements> end_iter(elements_vec,elements_vec.size());
  return boost::make_iterator_range(begin_iter,end_iter);
}

////////////////////////////////////////////////////////////////////////////////

Field& SpaceFields::field(const std::string& name) const
{
  return get_child(name).as_type<Field>();
}

////////////////////////////////////////////////////////////////////////////////

void SpaceFields::on_mesh_changed_event( SignalArgs& args )
{
  common::XML::SignalOptions options( args );


  URI mesh_uri = options.value<URI>("mesh_uri");
  if (mesh_uri.is_relative())
  {
    throw InvalidURI(FromHere(),"URI "+to_str(mesh_uri)+" should be absolute");
  }
  Mesh& mesh_arg = access_component(mesh_uri).as_type<Mesh>();

  Mesh& this_mesh = parent().as_type<Mesh>();

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

void SpaceFields::update()
{
  elements_lookup().reset();

  switch (m_basis)
  {
    case Basis::POINT_BASED:
    case Basis::ELEMENT_BASED:
      boost_foreach(Entities& entities, find_components_recursively<Entities>(topology()))
        elements_lookup().add(entities);
      break;
    case Basis::CELL_BASED:
      boost_foreach(Cells& cells, find_components_recursively<Cells>(topology()))
        elements_lookup().add(cells);
      break;
    case Basis::FACE_BASED:
      boost_foreach(Entities& faces, find_components_recursively_with_tag<Entities>(topology(),mesh::Tags::face_entity()))
        elements_lookup().add(faces);
      break;
    default:
      throw InvalidStructure(FromHere(), "basis not set");
  }

  bind_space();
//  if (m_basis != Basis::POINT_BASED)
//  {
//    Uint new_size = 0;
//    boost_foreach(Entities& entities, entities_range())
//      new_size += entities.space(m_space).nb_states() * entities.size();
//    resize(new_size);
//    bind_space();
//  }
//  else
//  {
//    if (m_space != Entities::MeshSpaces::to_str(Entities::MeshSpaces::MESH_NODES))
//    {
//      bind_space();
//    }
//  }

  check_sanity();
}

////////////////////////////////////////////////////////////////////////////////

void SpaceFields::bind_space()
{
  if (m_topology->is_linked() == false)
    throw SetupError(FromHere(), "topology of field_group ["+uri().string()+"] not configured");
  if (m_space == "invalid")
    throw SetupError(FromHere(), "space of field_group ["+uri().string()+"] not configured");
  if (m_basis == Basis::INVALID)
    throw SetupError(FromHere(), "type of field_group ["+uri().string()+"] not configured");

  if (m_space != mesh::Tags::geometry())
    create_connectivity_in_space();
  // else the connectivity must be manually created by mesh reader or mesh transformer

  boost_foreach(Entities& entities, entities_range())
    entities.space(m_space).get_child("fields").as_type<Link>().link_to(*this);
}

std::size_t hash_value(const RealMatrix& coords)
{
  std::size_t seed=0;
  for (Uint i=0; i<coords.rows(); ++i)
  for (Uint j=0; j<coords.cols(); ++j)
  {
    // multiply with 1e-5 (arbitrary) to avoid hash collisions
    boost::hash_combine(seed,1e-6*coords(i,j));
  }
  return seed;
}


void SpaceFields::create_connectivity_in_space()
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
    boost_foreach(Entities& entities, elements_range())
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

    // step 3: resize
    // --------------
    resize(points.size());
    m_coordinates = coordinates.as_ptr<Field>();
    for (Uint i=0; i<size(); ++i)
      rank()[i] = UNKNOWN;

    // step 2: collect nodes in a set
    // ------------------------------
    boost_foreach(Entities& entities, entities_range())
    {
      SpaceFields& geometry = entities.geometry_fields();
      Connectivity& geometry_node_connectivity = entities.geometry_space().connectivity();
      common::List<Uint>& geometry_rank = entities.geometry_fields().rank();
      entities.space(m_space).get_child("fields").as_type<Link>().link_to(*this);
      const ShapeFunction& shape_function = entities.space(m_space).shape_function();
      Connectivity& connectivity = entities.space(m_space).connectivity();
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
          rank()[idx] = UNKNOWN;
          glb_idx()[idx] = UNKNOWN;
        }
      }
    }

    // step 4: add lookup to connectivity tables
    // -----------------------------------------
    boost_foreach(Entities& entities, entities_range())
        entities.space(m_space).connectivity().create_lookup().add(*this);

    // step 5: fix unknown ranks
    // -------------------------
    cf3_assert(size() == m_rank->size());
    cf3_assert(size() == m_glb_idx->size());
    cf3_assert(size() == m_coordinates->size());

    boost_foreach(Entities& entities, entities_range())
    {
      Space& space = entities.space(m_space);
      for (Uint e=0; e<entities.size(); ++e)
      {
        Connectivity::ConstRow nodes = space.indexes_for_element(e);
        boost_foreach(const Uint node, nodes)
          rank()[node] = std::min(rank()[node] , entities.rank()[e] );
      }
    }

    std::map<size_t,Uint> hash_to_idx;
    std::map<size_t,Uint>::iterator hash_to_idx_iter;
    std::map<size_t,Uint>::iterator hash_not_found = hash_to_idx.end();

    std::vector<size_t> coord_hash(size());
    RealVector dummy(coordinates.row_size());
    Uint c(0);
    for (Uint i=0; i<size(); ++i)
    {
      for (Uint d=0; d<coordinates.row_size(); ++d)
        dummy[d] = coordinates[i][d];
      size_t hash = hash_value(dummy);
      hash_to_idx[hash] = i;
      coord_hash[c++]=hash;
    }

    // - Communicate unknown ranks to all processes
    std::vector< std::vector<size_t> > recv_hash(Comm::instance().size());
    if (Comm::instance().is_active())
      Comm::instance().all_gather(coord_hash,recv_hash);
    else
      recv_hash[0] = coord_hash;

    // - Search this process contains the missing ranks of other processes
    std::vector< std::vector<Uint> > send_found_on_rank(Comm::instance().size());
    for (Uint p=0; p<Comm::instance().size(); ++p)
    {
      send_found_on_rank[p].resize(recv_hash[p].size(),UNKNOWN);
      {
        for (Uint h=0; h<recv_hash[p].size(); ++h)
        {
          hash_to_idx_iter = hash_to_idx.find(recv_hash[p][h]);
          if ( hash_to_idx_iter != hash_not_found )
          {
            send_found_on_rank[p][h] = rank()[ hash_to_idx_iter->second ];
          }
        }
      }

    }

    // - Communicate which processes found the missing ranks
    std::vector< std::vector<Uint> > recv_found_on_rank(Comm::instance().size());
    if (Comm::instance().is_active())
      Comm::instance().all_to_all(send_found_on_rank,recv_found_on_rank);
    else
      recv_found_on_rank[0] = send_found_on_rank[0];


    // - Set the missing rank to the lowest process number that found it
    for (Uint n=0; n<size(); ++n)
    {
      Uint rank_that_owns = UNKNOWN;

      for (Uint p=0; p<Comm::instance().size(); ++p)
      {
        rank_that_owns = std::min(recv_found_on_rank[p][n], rank_that_owns);
      }
      rank()[n] = rank_that_owns;

      cf3_assert(rank()[n] != UNKNOWN);
    }

    // step 5: fix unknown glb_idx
    // ---------------------------
    Uint nb_owned = 0;
    std::deque<Uint> ghosts;
    for (Uint i=0; i<size(); ++i)
    {
      if (is_ghost(i))
        ghosts.push_back(i);
      else
        ++nb_owned;
    }
    std::vector<size_t> ghosts_hashed(ghosts.size());
    for (Uint g=0; g<ghosts.size(); ++g)
    {
      for (Uint d=0; d<coordinates.row_size(); ++d)
        dummy[d] = coordinates[ghosts[g]][d];
      ghosts_hashed[g] = hash_value(dummy);
    }
    std::vector<Uint> nb_owned_per_proc(Comm::instance().size(),nb_owned);
    if( Comm::instance().is_active() )
      Comm::instance().all_gather(nb_owned, nb_owned_per_proc);

    std::vector<Uint> start_id_per_proc(Comm::instance().size());

    Uint start_id=0;
    for (Uint p=0; p<Comm::instance().size(); ++p)
    {
      start_id_per_proc[p] = start_id;
      start_id += nb_owned_per_proc[p];
    }
    start_id = start_id_per_proc[Comm::instance().rank()];
    for (Uint i=0; i<size(); ++i)
    {
      if (! is_ghost(i))
        glb_idx()[i] = start_id++;
      else
        glb_idx()[i] = UNKNOWN;
    }

    std::vector< std::vector<size_t> > recv_ghosts_hashed(Comm::instance().size());
    if (Comm::instance().is_active())
      Comm::instance().all_gather(ghosts_hashed,recv_ghosts_hashed);
    else
      recv_ghosts_hashed[0] = ghosts_hashed;

    // - Search this process contains the missing ranks of other processes
    std::vector< std::vector<Uint> > send_glb_idx_on_rank(Comm::instance().size());
    for (Uint p=0; p<Comm::instance().size(); ++p)
    {
      send_glb_idx_on_rank[p].resize(recv_ghosts_hashed[p].size(),UNKNOWN);
      if (p!=Comm::instance().rank())
      {
        for (Uint h=0; h<recv_ghosts_hashed[p].size(); ++h)
        {
          hash_to_idx_iter = hash_to_idx.find(recv_ghosts_hashed[p][h]);
          if ( hash_to_idx_iter != hash_not_found )
          {
            send_glb_idx_on_rank[p][h] = glb_idx()[hash_to_idx_iter->second];
          }
        }
      }
    }

    // - Communicate which processes found the missing ghosts
    std::vector< std::vector<Uint> > recv_glb_idx_on_rank(Comm::instance().size());
    if (Comm::instance().is_active())
      Comm::instance().all_to_all(send_glb_idx_on_rank,recv_glb_idx_on_rank);
    else
      recv_glb_idx_on_rank[0] = send_glb_idx_on_rank[0];

    // - Set the missing rank to the lowest process number that found it
    for (Uint g=0; g<ghosts.size(); ++g)
    {
      cf3_assert(rank()[ghosts[g]] < Comm::instance().size());
      glb_idx()[ghosts[g]] = recv_glb_idx_on_rank[rank()[ghosts[g]]][g];
    }
  }
  else // If Element-based
  {
    // Check if this space is not already bound to another field_group
    boost_foreach(Entities& entities, entities_range())
    {
      if (entities.space(m_space).is_bound_to_fields() > 0)
        throw SetupError(FromHere(), "Space ["+entities.space(m_space).uri().string()+"] is already bound to\n"
                         "fields ["+entities.space(m_space).fields().uri().string()+"]\nCreate a new space for field_group ["+uri().string()+"]");
    }

    // Assign the space connectivity table
    Uint field_idx = 0;
    boost_foreach(Entities& entities, entities_range())
    {
      Space& space = entities.space(m_space);
      space.get_child("fields").as_type<Link>().link_to(*this);
      space.make_proxy(field_idx);
      field_idx += entities.size()*space.nb_states();
    }

    resize(field_idx);

    boost_foreach(Entities& entities, entities_range())
    {
      cf3_assert_desc("mesh not properly constructed",entities.rank().size() == entities.size());
      Space& space = entities.space(m_space);
      space.connectivity().create_lookup().add(*this);
      for (Uint e=0; e<entities.size(); ++e)
      {
        Connectivity::ConstRow field_indexes = space.indexes_for_element(e);
        boost_foreach(const Uint idx, field_indexes)
          rank()[idx] = entities.rank()[e];
      }
    }

    // step 5: fix unknown glb_idx
    // ---------------------------

    std::map<size_t,Uint> hash_to_elem_idx;
    std::map<size_t,Uint>::iterator hash_to_elem_idx_iter;
    std::map<size_t,Uint>::iterator hash_not_found = hash_to_elem_idx.end();

    Uint nb_owned(0);
    boost_foreach(const Entities& entities, entities_range())
    {
      Uint nb_states_per_cell = entities.space(m_space).nb_states();
      for (Uint e=0; e<entities.size(); ++e)
      {
        if (entities.is_ghost(e) == false)
          nb_owned+=nb_states_per_cell;
      }
    }

    std::vector<Uint> nb_owned_per_proc(Comm::instance().size(),nb_owned);
    if (Comm::instance().is_active())
      Comm::instance().all_gather(nb_owned, nb_owned_per_proc);

    std::vector<Uint> start_id_per_proc(Comm::instance().size(),0);
    for (Uint i=0; i<Comm::instance().size(); ++i)
    {
      start_id_per_proc[i] = (i==0? 0 : start_id_per_proc[i-1]+nb_owned_per_proc[i-1]);
    }

    Uint id = start_id_per_proc[Comm::instance().rank()];
    boost_foreach(const Entities& entities, entities_range())
    {
      Space& space = entities.space(m_space);
      for (Uint e=0; e<entities.size(); ++e)
      {
        if (entities.is_ghost(e) == false)
        {
          boost_foreach(const Uint idx, space.indexes_for_element(e))
            glb_idx()[idx] = id++;
        }
        else
        {
          boost_foreach(const Uint idx, space.indexes_for_element(e))
            glb_idx()[idx] = UNKNOWN;
        }
      }
    }

    boost_foreach(const Entities& entities, entities_range())
    {
      const Space& space = entities.space(m_space);
      Uint nb_states_per_elem = space.nb_states();
      std::deque<Uint> ghosts;
      std::deque<size_t> ghosts_hashed_deque;
      RealMatrix dummy(entities.geometry_space().nb_states(),entities.element_type().dimension());
      for (Uint e=0; e<entities.size(); ++e)
      {
        entities.put_coordinates(dummy,e);
        /// @bug entities.geometry_space().put_coordinates(dummy,e)  does not give same result as previous line
        size_t hash = hash_value(dummy);
        bool inserted = hash_to_elem_idx.insert( std::make_pair(hash, e) ).second;
        if (! inserted)
        {
          std::stringstream msg;
          msg <<"Duplicate hash " << hash << " detected for coords \n" << dummy;
          throw ValueExists(FromHere(), msg.str());
        }
        if (entities.is_ghost(e))
        {
          ghosts.push_back(e);
          ghosts_hashed_deque.push_back(hash);
        }
      }

      // copy deque in vector, delete deque
      std::vector<size_t> ghosts_hashed(ghosts.size());
      for (Uint g=0; g<ghosts.size(); ++g)
      {
        ghosts_hashed[g] = ghosts_hashed_deque[g];
      }
      ghosts_hashed_deque.clear();

      std::vector< std::vector<size_t> > recv_ghosts_hashed(Comm::instance().size());
      if (Comm::instance().is_active())
        Comm::instance().all_gather(ghosts_hashed,recv_ghosts_hashed);
      else
        recv_ghosts_hashed[0] = ghosts_hashed;

      // - Search this process contains the unknown ghosts of other processes
      std::vector< std::vector<Uint> > send_glb_idx_on_rank(Comm::instance().size());
      for (Uint p=0; p<Comm::instance().size(); ++p)
      {
        send_glb_idx_on_rank[p].resize(recv_ghosts_hashed[p].size(),UNKNOWN);
        if (p!=Comm::instance().rank())
        {
          for (Uint h=0; h<recv_ghosts_hashed[p].size(); ++h)
          {
            hash_to_elem_idx_iter = hash_to_elem_idx.find(recv_ghosts_hashed[p][h]);
            if ( hash_to_elem_idx_iter != hash_not_found )
            {
              Uint first_glb_idx = glb_idx()[ space.indexes_for_element( hash_to_elem_idx_iter->second )[0] ];
              send_glb_idx_on_rank[p][h] = first_glb_idx;
            }
          }
        }
      }


      // - Communicate which processes found the missing ghosts
      std::vector< std::vector<Uint> > recv_glb_idx_on_rank(Comm::instance().size());
      if (Comm::instance().is_active())
        Comm::instance().all_to_all(send_glb_idx_on_rank,recv_glb_idx_on_rank);
      else
        recv_glb_idx_on_rank[0] = send_glb_idx_on_rank[0];


      // - Set the missing rank to the lowest process number that found it
      for (Uint g=0; g<ghosts.size(); ++g)
      {
        cf3_assert(entities.rank()[ghosts[g]] < Comm::instance().size());
        const Uint first_loc_idx = space.indexes_for_element(ghosts[g])[0];
        const Uint first_glb_idx = recv_glb_idx_on_rank[ entities.rank()[ghosts[g]] ][g];
        for (Uint s=0; s<nb_states_per_elem; ++s)
          glb_idx()[first_loc_idx+s] = first_glb_idx+s;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

common::Table<Uint>::ConstRow SpaceFields::indexes_for_element(const Entities& elements, const Uint idx) const
{
  Space& space = elements.space(m_space);
  cf3_assert_desc("space not bound to this field_group", &space.fields() == this);
  return space.indexes_for_element(idx);
}

////////////////////////////////////////////////////////////////////////////////

common::Table<Uint>::ConstRow SpaceFields::indexes_for_element(const Uint unified_idx) const
{
  Component::Ptr component;
  Uint elem_idx;
  boost::tie(component,elem_idx) = elements_lookup().location(unified_idx);
  return indexes_for_element(component->as_type<Entities>(),elem_idx);
}

////////////////////////////////////////////////////////////////////////////////

bool SpaceFields::has_coordinates() const
{
  return is_not_null(m_coordinates);
}

////////////////////////////////////////////////////////////////////////////////

Field& SpaceFields::coordinates()
{
  if (is_null(m_coordinates))
  {
    if (Field::Ptr found = find_component_ptr_with_tag<Field>(*this,mesh::Tags::coordinates()))
    {
      m_coordinates = found;
    }
    else
    {
      throw ValueNotFound(FromHere(),"SpaceFields ["+uri().string()+"] has no coordinates field");
    }
  }
  return *m_coordinates;
}

////////////////////////////////////////////////////////////////////////////////

const Field& SpaceFields::coordinates() const
{
  if (is_null(m_coordinates))
    throw ValueNotFound(FromHere(),"SpaceFields ["+uri().string()+"] has no coordinates field");
  return *m_coordinates;
}

////////////////////////////////////////////////////////////////////////////////

Field& SpaceFields::create_coordinates()
{
  if (has_coordinates())
    throw ValueExists(FromHere(),"coordinates cannot be created, they already exist");

  Field& coordinates = create_field("coordinates","coords[vector]");
  boost_foreach(Entities& entities, entities_range())
  {
    Space& geometry_space = entities.geometry_space();
    const ShapeFunction& geom_sf = geometry_space.shape_function();
    RealMatrix geom_nodes;
    entities.allocate_coordinates(geom_nodes);

    Space& space = entities.space(m_space);
    const ShapeFunction& sf = space.shape_function();
    const RealMatrix& local_coords = sf.local_coordinates();

    RealMatrix interpolation(sf.nb_nodes(),geom_sf.nb_nodes());
    for (Uint i=0; i<sf.nb_nodes(); ++i)
      interpolation.row(i) = geom_sf.value(local_coords.row(i));

    RealMatrix coords(sf.nb_nodes(),geom_nodes.cols());

    for (Uint e=0; e<entities.size(); ++e)
    {
      Connectivity::ConstRow field_index = space.indexes_for_element(e);

      entities.put_coordinates(geom_nodes,e);
      coords = interpolation*geom_nodes;

      for (Uint i=0; i<sf.nb_nodes(); ++i)
      {
        const Uint pt = field_index[i];
        for(Uint d=0; d<coords.cols(); ++d)
          coordinates[pt][d] = coords(i,d);
      }
    }
  }

  m_coordinates = coordinates.as_ptr<Field>();
  return coordinates;
}

////////////////////////////////////////////////////////////////////////////////

DynTable<Uint>& SpaceFields::glb_elem_connectivity()
{
  if (is_null(m_glb_elem_connectivity))
  {
    m_glb_elem_connectivity = create_static_component_ptr< DynTable<Uint> >("glb_elem_connectivity");
    m_glb_elem_connectivity->add_tag("glb_elem_connectivity");
    m_glb_elem_connectivity->resize(size());
  }
  return *m_glb_elem_connectivity;
}



////////////////////////////////////////////////////////////////////////////////

void SpaceFields::signature_create_field( SignalArgs& node )
{
  SignalOptions options( node );

  options.add_option< OptionT<std::string> >("name")
      ->description("Name of the field" );

  options.add_option< OptionT<std::string> >("variables")
      ->description("Variables description of the field" );

}

////////////////////////////////////////////////////////////////////////////////

void SpaceFields::signal_create_field( SignalArgs& node )
{
  SignalOptions options( node );

  std::string name = options.value<std::string>("name");
  std::string variables = name;
  if(options.check("variables"))
  {
    variables = options.value<std::string>("variables");
  }
  create_field(name,variables);
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

#undef UNKNOWN
