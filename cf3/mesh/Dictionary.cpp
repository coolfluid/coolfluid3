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
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/Builder.hpp"
#include "common/Link.hpp"
#include "common/FindComponents.hpp"
#include "common/Core.hpp"
#include "common/EventHandler.hpp"
#include "common/StringConversion.hpp"
#include "common/Tags.hpp"
#include "common/DynTable.hpp"
#include "common/XML/SignalOptions.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/CommPattern.hpp"
#include "common/PE/debug.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/LibMesh.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Dictionary.hpp"
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

#include "common/OptionList.hpp"

#include "common/OptionList.hpp"

namespace cf3 {
namespace mesh {

using namespace boost::assign;

using namespace common;
using namespace common::PE;
using namespace common::XML;

common::ComponentBuilder < Dictionary, Component, LibMesh >  Dictionary_Builder;

////////////////////////////////////////////////////////////////////////////////

Dictionary::Basis::Convert::Convert()
{
  all_fwd = boost::assign::map_list_of
      ( Dictionary::Basis::POINT_BASED, "point_based" )
      ( Dictionary::Basis::ELEMENT_BASED, "element_based" )
      ( Dictionary::Basis::CELL_BASED, "cell_based" )
      ( Dictionary::Basis::FACE_BASED, "face_based" );

  all_rev = boost::assign::map_list_of
      ("point_based",    Dictionary::Basis::POINT_BASED )
      ("element_based",  Dictionary::Basis::ELEMENT_BASED )
      ("cell_based",     Dictionary::Basis::CELL_BASED )
      ("face_based",     Dictionary::Basis::FACE_BASED );
}

////////////////////////////////////////////////////////////////////////////////

Dictionary::Basis::Convert& Dictionary::Basis::Convert::instance()
{
  static Dictionary::Basis::Convert instance;
  return instance;
}

////////////////////////////////////////////////////////////////////////////////

Dictionary::Dictionary ( const std::string& name  ) :
  Component( name ),
  m_basis(Basis::INVALID),
  m_size(0u)
{
  mark_basic();

  // Option "type"
  options().add_option("type", Basis::to_str(m_basis))
      .description("The type of the field")
      .attach_trigger ( boost::bind ( &Dictionary::config_type,   this ) )
      .mark_basic()
      .restricted_list() =  list_of
      (Basis::to_str(Basis::POINT_BASED))
      (Basis::to_str(Basis::ELEMENT_BASED))
      (Basis::to_str(Basis::CELL_BASED))
      (Basis::to_str(Basis::FACE_BASED));

  // Static components

  m_rank = create_static_component< common::List<Uint> >("rank");
  m_rank->add_tag("rank");

  m_glb_idx = create_static_component< common::List<Uint> >(mesh::Tags::global_indices());
  m_glb_idx->add_tag(mesh::Tags::global_indices());


  // Event handlers
//  Core::instance().event_handler().connect_to_event("mesh_loaded", this, &Dictionary::on_mesh_changed_event);
  Core::instance().event_handler().connect_to_event("mesh_changed", this, &Dictionary::on_mesh_changed_event);

  // Signals
  regist_signal ( "create_field" )
      .description( "Create Field" )
      .pretty_name("Create Field" )
      .connect   ( boost::bind ( &Dictionary::signal_create_field,    this, _1 ) )
      .signature ( boost::bind ( &Dictionary::signature_create_field, this, _1 ) );


}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::config_type()
{
  m_basis = Basis::to_enum( options().option("type").value<std::string>() );
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::add_space(const Handle<Space>& space)
{
  m_spaces_map.insert( std::make_pair(space->support().handle<Entities>(),space ) );
}

////////////////////////////////////////////////////////////////////////////////

Dictionary::~Dictionary()
{
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::resize(const Uint size)
{
  m_size = size;
  m_glb_idx->resize(m_size);
  m_rank->resize(m_size);
  properties()["size"]=m_size;

  boost_foreach(Field& field, find_components<Field>(*this))
    field.resize(m_size);
}

//////////////////////////////////////////////////////////////////////////////

CommPattern& Dictionary::comm_pattern()
{
  if(is_null(m_comm_pattern))
  {
    PE::CommPattern& comm_pattern = *create_component<PE::CommPattern>("CommPattern");
    comm_pattern.insert("gid",glb_idx().array(),false);
    comm_pattern.setup(Handle<PE::CommWrapper>(comm_pattern.get_child("gid")),rank().array());
    m_comm_pattern = Handle<common::PE::CommPattern>(comm_pattern.handle<Component>());
  }

  return *m_comm_pattern;
}

//////////////////////////////////////////////////////////////////////////////

bool Dictionary::is_ghost(const Uint idx) const
{
  cf3_assert_desc(to_str(idx)+">="+to_str(size()),idx < size());
  cf3_assert(size() == m_rank->size());
  cf3_assert(idx<m_rank->size());
  return (*m_rank)[idx] != Comm::instance().rank();
}

////////////////////////////////////////////////////////////////////////////////

const Space& Dictionary::space(const Entities& entities) const
{
  return *space(entities.handle<Entities>());
}

////////////////////////////////////////////////////////////////////////////////

const Handle< Space const>& Dictionary::space(const Handle<Entities const>& entities) const
{
  const Handle< Space const>& s = m_spaces_map.find(entities)->second;
  cf3_assert(s);
  return s;
}

////////////////////////////////////////////////////////////////////////////////

Field& Dictionary::create_field(const std::string &name, const std::string& variables_description)
{

  Field& field = *create_component<Field>(name);
  field.set_dict(*this);
  field.set_basis(m_basis);

  if (variables_description == "scalar_same_name")
    field.create_descriptor(name+"[scalar]",Handle<Mesh>(parent())->dimension());
  else
    field.create_descriptor(variables_description,Handle<Mesh>(parent())->dimension());

  field.resize(m_size);
  return field;
}

////////////////////////////////////////////////////////////////////////////////

Field& Dictionary::create_field(const std::string &name, math::VariablesDescriptor& variables_descriptor)
{
  Field& field = *create_component<Field>(name);
  field.set_dict(*this);
  field.set_basis(m_basis);
  field.set_descriptor(variables_descriptor);
  if (variables_descriptor.options().option(common::Tags::dimension()).value<Uint>() == 0)
    field.descriptor().options().configure_option(common::Tags::dimension(),Handle<Mesh>(parent())->dimension());
  field.resize(m_size);
  return field;
}

////////////////////////////////////////////////////////////////////////////////

bool Dictionary::check_sanity(std::vector<std::string>& messages) const
{
  Uint nb_messages_init = messages.size();
  if (rank().size() != size())
    messages.push_back(uri().string()+": size() ["+to_str(size())+"] != rank().size() ["+to_str(rank().size())+"]");

  if (glb_idx().size() != size())
    messages.push_back(uri().string()+": size() ["+to_str(size())+"] != glb_idx().size() ["+to_str(glb_idx().size())+"]");

  std::set<Uint> unique_gids;
  if (Comm::instance().size()>1)
  {
    for (Uint i=0; i<size(); ++i)
    {
      if (rank()[i] >= PE::Comm::instance().size())
      {
        messages.push_back(rank().uri().string()+"["+to_str(i)+"] has invalid entry. (entry = "+to_str(rank()[i])+" , no further checks)");
        break;
      }
    }
    for (Uint i=0; i<size(); ++i)
    {
      std::pair<std::set<Uint>::iterator, bool > inserted = unique_gids.insert(glb_idx()[i]);
      if (inserted.second == false)
      {
        messages.push_back(glb_idx().uri().string()+"["+to_str(i)+"] has non-unique entries.  (entry "+to_str(glb_idx()[i])+" exists more than once, no further checks)");
        break;
      }
    }
  }


  boost_foreach(const Field& field, find_components_recursively<Field>(*this))
  {
    if (field.size() != size())
      messages.push_back(uri().string()+": size() ["+to_str(size())+"] != "+field.uri().string()+".size() ["+to_str(field.size())+"]");
  }
  // Sane if number of messages did not grow in size
  return messages.size() == nb_messages_init;
}

bool Dictionary::check_sanity() const
{
  std::vector<std::string> messages;
  bool sane = check_sanity(messages);
  if ( sane == false )
  {
    std::stringstream message;
    message << "Dictionary "+uri().string()+" is not sane:"<<std::endl;
    boost_foreach(const std::string& str, messages)
    {
      message << "- " << str << std::endl;
    }
    throw InvalidStructure(FromHere(), message.str() );
  }
  return sane;
}

////////////////////////////////////////////////////////////////////////////////

std::vector< Handle< Entities > > Dictionary::entities_range()
{
//  std::vector<Handle< Entities > > elements_vec(elements_lookup().components().size());
//  for (Uint c=0; c<elements_vec.size(); ++c)
//    elements_vec[c] = Handle<Entities>(elements_lookup().components()[c]);

  return m_entities;
}

////////////////////////////////////////////////////////////////////////////////

//std::vector< Handle< Elements > > Dictionary::elements_range()
//{
//  std::vector<Handle< Elements > > elements_vec(elements_lookup().components().size());
//  for (Uint c=0; c<elements_vec.size(); ++c)
//    elements_vec[c] = Handle<Elements>(elements_lookup().components()[c]);

//  return elements_vec;
//}

////////////////////////////////////////////////////////////////////////////////

Field& Dictionary::field(const std::string& name)
{
  cf3_assert_desc("field "+name+" not found in "+uri().string(),get_child(name));
  return *Handle<Field>(get_child(name));
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::on_mesh_changed_event( SignalArgs& args )
{
  common::XML::SignalOptions options( args );


  URI mesh_uri = options.value<URI>("mesh_uri");
  if (mesh_uri.is_relative())
  {
    throw InvalidURI(FromHere(),"URI "+to_str(mesh_uri)+" should be absolute");
  }
  Mesh& mesh_arg  = *Handle<Mesh>(access_component(mesh_uri));
  Mesh& this_mesh = *Handle<Mesh>(parent());

  if (&this_mesh == &mesh_arg)
  {
    if (m_spaces_map.size())
      update();
  }
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::update()
{
  m_entities.clear();
  m_spaces.clear();
  m_entities.reserve(m_spaces_map.size());
  m_spaces.reserve(m_spaces_map.size());

  std::vector< Handle<Entities const> > to_remove_from_spaces_map;
  to_remove_from_spaces_map.reserve(m_spaces_map.size());
  foreach_container( (const Handle<Entities const>& entities) (const Handle<Space const>& space), m_spaces_map)
  {
    if ( is_null(entities) )
    {
      to_remove_from_spaces_map.push_back(entities);
    }
    else
    {
      cf3_assert(space);
      m_entities.push_back(const_cast<Entities&>(*entities).handle<Entities>());
      m_spaces.push_back(const_cast<Space&>(*space).handle<Space>());
    }
  }
  boost_foreach( const Handle<Entities const>& entities, to_remove_from_spaces_map)
  {
    m_spaces_map.erase(entities);
  }

  if(has_tag(mesh::Tags::geometry()) == false)
    create_connectivity_in_space();
  check_sanity();
}

////////////////////////////////////////////////////////////////////////////////

bool Dictionary::defined_for_entities(const Handle<Entities const>& entities) const
{
  return ( m_spaces_map.find(entities) != m_spaces_map.end() );
}

////////////////////////////////////////////////////////////////////////////////

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


void Dictionary::create_connectivity_in_space()
{
  if (m_basis == Basis::INVALID)
    throw SetupError(FromHere(), "type of dict ["+uri().string()+"] not configured");


  if (m_basis == Basis::POINT_BASED)
  {
    std::set<std::size_t> points;
    RealMatrix elem_coordinates;
    Uint dim = DIM_0D;

    // step 1: collect nodes in a set
    // ------------------------------
    boost_foreach(const Handle<Entities>& entities_handle, entities_range())
    {
      Entities& entities = *entities_handle;
      const ShapeFunction& shape_function = space(entities).shape_function();
      for (Uint elem=0; elem<entities.size(); ++elem)
      {
        elem_coordinates = entities.geometry_space().get_coordinates(elem);
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
    m_coordinates = Handle<Field>(coordinates.handle<Component>());
    for (Uint i=0; i<size(); ++i)
      rank()[i] = UNKNOWN;

    // step 2: collect nodes in a set
    // ------------------------------
    boost_foreach(const Handle<Entities>& entities_handle, entities_range())
    {
      Entities& entities = *entities_handle;
      Dictionary& geometry = entities.geometry_fields();
      Connectivity& geometry_node_connectivity = entities.geometry_space().connectivity();
      common::List<Uint>& geometry_rank = entities.geometry_fields().rank();
      const ShapeFunction& shape_function = space(entities).shape_function();
      Connectivity& connectivity = const_cast<Space&>(space(entities)).connectivity();
      connectivity.resize(entities.size());
      for (Uint elem=0; elem<entities.size(); ++elem)
      {
        elem_coordinates = entities.geometry_space().get_coordinates(elem);
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

    // step 5: fix unknown ranks
    // -------------------------
    cf3_assert(size() == m_rank->size());
    cf3_assert(size() == m_glb_idx->size());
    cf3_assert(size() == m_coordinates->size());

    boost_foreach(const Handle<Entities>& entities_handle, entities_range())
    {
      Entities& entities = *entities_handle;
      const Connectivity& space_connectivity = space(entities).connectivity();
      for (Uint e=0; e<entities.size(); ++e)
      {
        boost_foreach(const Uint node, space_connectivity[e])
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
    // STEP 1: Assign the space connectivity table in the topology
    // -----------------------------------------------------------
    Uint field_idx = 0;
    boost_foreach(const Handle<Entities>& entities, entities_range())
    {
      const Space& entities_space = *space(entities);
      Connectivity& space_connectivity = const_cast<Connectivity&>(entities_space.connectivity());
      space_connectivity.resize(entities->size());
      Uint nb_nodes = entities_space.nb_states();
      for (Uint elem=0; elem<entities->size(); ++elem)
      {
        for (Uint node=0; node<nb_nodes; ++node)
          space_connectivity[elem][node] = field_idx++;
      }
    }

    // STEP 2: Resize this space
    // -------------------------
    resize(field_idx);

    // STEP 3: fix the unknown ranks of this space
    // -------------------------------------------
    //  - The rank of every node will be the rank of the element it belongs to
    boost_foreach(const Handle<Entities>& entities_handle, entities_range())
    {
      Entities& entities = *entities_handle;
      cf3_assert_desc("mesh not properly constructed",entities.rank().size() == entities.size());
      const Connectivity& space_connectivity = space(entities).connectivity();
      for (Uint e=0; e<entities.size(); ++e)
      {
        boost_foreach(const Uint idx, space_connectivity[e])
        {
//          cf3_assert(entities.rank()[e] < PE::Comm::instance().size());
          rank()[idx] = entities.rank()[e];
        }
      }
    }

    // (3)
    std::map<size_t,Entity> hash_to_elements;

    std::deque<Entity> unknown_rank_elements;
    std::deque<size_t> unknown_rank_elements_hash_deque;

    boost_foreach(const Handle<Entities>& entities_handle, entities_range())
    {
      Entities& entities = *entities_handle;
      const Space& entities_space = space(entities);
      Uint nb_states_per_elem = entities_space.nb_states();
      RealMatrix elem_coords(entities.geometry_space().nb_states(),entities.element_type().dimension());
      RealVector centroid(entities.element_type().dimension());
      for (Uint e=0; e<entities.size(); ++e)
      {
        entities.geometry_space().put_coordinates(elem_coords,e);
        entities.element_type().compute_centroid(elem_coords,centroid);
        size_t hash = hash_value(centroid);
//        std::cout << "["<<PE::Comm::instance().rank() << "]  hashed "<< entities.uri().path() << "["<<e<<"]) to " << hash;
        bool inserted = hash_to_elements.insert( std::make_pair(hash, Entity(entities,e)) ).second;
        if (! inserted)
        {
          std::stringstream msg;
          msg <<"Duplicate hash " << hash << " detected for element with centroid \n" << centroid;
          throw ValueExists(FromHere(), msg.str());
        }
        if(entities.rank()[e] == UNKNOWN)
        {
          unknown_rank_elements.push_back(Entity(entities,e));
          unknown_rank_elements_hash_deque.push_back(hash);
        }
//        std::cout << std::endl;
      }
    }

    // copy deque in vector, delete deque
    std::vector<size_t> unknown_rank_elements_hashed(unknown_rank_elements_hash_deque.size());
    for (Uint g=0; g<unknown_rank_elements_hash_deque.size(); ++g)
    {
      unknown_rank_elements_hashed[g] = unknown_rank_elements_hash_deque[g];
    }
    unknown_rank_elements_hash_deque.clear();


    // (4)
    std::vector< std::vector<size_t> > recv_unknown_rank_elements_hashed(Comm::instance().size());
    if (Comm::instance().is_active())
      Comm::instance().all_gather(unknown_rank_elements_hashed,recv_unknown_rank_elements_hashed);
    else
      recv_unknown_rank_elements_hashed[0] = unknown_rank_elements_hashed;

    // (5) Search if this process contains the elements with unknown ranks of other processes
    std::map<size_t,Entity>::iterator hash_to_elements_iter;
    std::map<size_t,Entity>::iterator hash_not_found = hash_to_elements.end();
    std::vector< std::vector<Uint> > send_found_on_rank(Comm::instance().size());
    for (Uint p=0; p<Comm::instance().size(); ++p)
    {
      send_found_on_rank[p].resize(recv_unknown_rank_elements_hashed[p].size(),UNKNOWN);
      if (p!=Comm::instance().rank())
      {
        for (Uint h=0; h<recv_unknown_rank_elements_hashed[p].size(); ++h)
        {
//          std::cout << PERank << "looking for hash " << recv_unknown_rank_elements_hashed[p][h] << " for proc " << p ;
          hash_to_elements_iter = hash_to_elements.find(recv_unknown_rank_elements_hashed[p][h]);
          if ( hash_to_elements_iter != hash_not_found )
          {
            const Space& entities_space = space(*hash_to_elements_iter->second.comp);
            const Uint elem_idx = hash_to_elements_iter->second.idx;
            if (rank()[entities_space.connectivity()[elem_idx][0]] == PE::Comm::instance().rank())
            {
              send_found_on_rank[p][h] = PE::Comm::instance().rank();
//              std::cout << " ---> found " ;
            }
            else
            {
              send_found_on_rank[p][h] = UNKNOWN-1;
//              std::cout << " ---> found but unknown" ;
            }
          }
//          std::cout << std::endl;
        }
      }
      else
      {
        for (Uint h=0; h<recv_unknown_rank_elements_hashed[p].size(); ++h)
          send_found_on_rank[p][h] = UNKNOWN-1;
      }
    }

    // (6)
    std::vector< std::vector<Uint> > recv_found_on_rank(Comm::instance().size());
    if (Comm::instance().is_active())
      Comm::instance().all_to_all(send_found_on_rank,recv_found_on_rank);
    else
      recv_found_on_rank[0] = send_found_on_rank[0];

    for (Uint g=0; g<unknown_rank_elements.size(); ++g)
    {
      const Space& entities_space = space(*unknown_rank_elements[g].comp);
      const Uint elem_idx = unknown_rank_elements[g].idx;
      cf3_assert(elem_idx<entities_space.connectivity().size());
      cf3_assert(elem_idx<entities_space.support().rank().size());

      const Uint first_loc_idx = entities_space.connectivity()[elem_idx][0];
      Uint recv_rank = UNKNOWN;
      Uint found_rank = UNKNOWN;
//      std::cout << PERank << "checking hash " << unknown_rank_elements_hashed[g] << ": ";
      for (Uint p=0; p<Comm::instance().size(); ++p)
      {
//        std::cout << recv_found_on_rank[p][g] << "   ";
        if ( recv_found_on_rank[p][g] < recv_rank )
        {
          recv_rank = recv_found_on_rank[p][g];
          found_rank = p;
        }
      }
//      std::cout << std::endl;
      if (found_rank == UNKNOWN) // ---> Nobody owns this. This is because it is notfound in
        found_rank = PE::Comm::instance().rank();
//        throw ValueNotFound(FromHere(), "Could not find rank for element "+entities_space.uri().path()+"["+to_str(elem_idx)+"] with hash "+to_str(unknown_rank_elements_hashed[g]));

//      std::cout << PERank << "set unknown element rank: " << unknown_rank_elements_hashed[g] << " --> " << found_rank << std::endl;

      for (Uint s=0; s<entities_space.nb_states(); ++s)
      {
        cf3_assert(first_loc_idx+s<rank().size());
        rank()[first_loc_idx+s] = found_rank;
      }
    }


    // STEP 4: fix unknown glb_idx
    // ---------------------------
    //  (1) Count the number of owned entries per process (owned when element it belongs to is owned)
    //  (2) glb_idx is filled in, ghost-entries are marked by a value "UNKNOWN" (=uint_max)
    //  (3) Create map< hash-value , element > of all elements. It will be used to match different cpu-elems
    //  (4) hash-values of ghost elements entries are communicated for lookup
    //  (5) lookup of received hash-values from other processes are translated into owned glb_idx of space-entries
    //  (6) glb_idx of ghost entries are received back


    // (1)

    Uint nb_owned(0);
    boost_foreach(const Handle<Entities>& entities_handle, entities_range())
    {
      Entities& entities = *entities_handle;
      Uint nb_states_per_cell = space(entities).nb_states();

      for (Uint e=0; e<entities.size(); ++e)
      {
        if (rank()[space(entities_handle)->connectivity()[e][0]] == PE::Comm::instance().rank())
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

    // (2)
    Uint id = start_id_per_proc[Comm::instance().rank()];
    boost_foreach(const Handle<Entities>& entities_handle, entities_range())
    {
      Entities& entities = *entities_handle;
      const Connectivity& space_connectivity = space(entities).connectivity();
      for (Uint e=0; e<entities.size(); ++e)
      {
        if (rank()[space_connectivity[e][0]] == PE::Comm::instance().rank())
        {
          boost_foreach(const Uint idx, space_connectivity[e])
            glb_idx()[idx] = id++;
        }
        else
        {
          boost_foreach(const Uint idx, space_connectivity[e])
            glb_idx()[idx] = UNKNOWN;
        }
      }
    }

    // (3)
    std::deque<Entity> ghosts;
    std::deque<size_t> ghosts_hashed_deque;

    boost_foreach(const Handle<Entities>& entities_handle, entities_range())
    {
      Entities& entities = *entities_handle;
      const Space& entities_space = space(entities);
      Uint nb_states_per_elem = entities_space.nb_states();
      RealMatrix elem_coords(entities.geometry_space().nb_states(),entities.element_type().dimension());
      RealVector centroid(entities.element_type().dimension());
      for (Uint e=0; e<entities.size(); ++e)
      {
        entities.geometry_space().put_coordinates(elem_coords,e);
        entities.element_type().compute_centroid(elem_coords,centroid);
        size_t hash = hash_value(centroid);
//        std::cout << "["<<PE::Comm::instance().rank() << "]  hashed "<< entities.uri().path() << "["<<e<<"]) to " << hash;
        if (rank()[entities_space.connectivity()[e][0]] != PE::Comm::instance().rank()) // if is ghost
        {
          cf3_assert(rank()[entities_space.connectivity()[e][0]] != UNKNOWN);
//          std::cout << "    --> is ghost, owned by " << entities.rank()[e] ;
          ghosts.push_back(Entity(entities,e));
          ghosts_hashed_deque.push_back(hash);
        }
//        std::cout << std::endl;
      }
    }

    // copy deque in vector, delete deque
    std::vector<size_t> ghosts_hashed(ghosts.size());
    for (Uint g=0; g<ghosts.size(); ++g)
    {
      ghosts_hashed[g] = ghosts_hashed_deque[g];
    }
    ghosts_hashed_deque.clear();

    // (4)
    std::vector< std::vector<size_t> > recv_ghosts_hashed(Comm::instance().size());
    if (Comm::instance().is_active())
      Comm::instance().all_gather(ghosts_hashed,recv_ghosts_hashed);
    else
      recv_ghosts_hashed[0] = ghosts_hashed;

    // (5) Search if this process contains the unknown ghosts of other processes
    std::vector< std::vector<Uint> > send_glb_idx_on_rank(Comm::instance().size());
    for (Uint p=0; p<Comm::instance().size(); ++p)
    {
      send_glb_idx_on_rank[p].resize(recv_ghosts_hashed[p].size(),UNKNOWN);
      if (p!=Comm::instance().rank())
      {
        for (Uint h=0; h<recv_ghosts_hashed[p].size(); ++h)
        {
//          std::cout << PERank << "looking for hash " << recv_ghosts_hashed[p][h] << " for proc " << p ;
          hash_to_elements_iter = hash_to_elements.find(recv_ghosts_hashed[p][h]);
          if ( hash_to_elements_iter != hash_not_found )
          {
//            std::cout << " ---> found " ;
            const Space& entities_space = space(*hash_to_elements_iter->second.comp);
            const Uint elem_idx = hash_to_elements_iter->second.idx;

            if (rank()[entities_space.connectivity()[elem_idx][0]] == PE::Comm::instance().rank()) // if owned
            {
              cf3_assert_desc(to_str(hash_to_elements_iter->second.idx)+" < "+to_str(entities_space.connectivity().size()),
                              hash_to_elements_iter->second.idx < entities_space.connectivity().size());
              cf3_assert(entities_space.connectivity()[ elem_idx ][0] < glb_idx().size());
              Uint first_glb_idx = glb_idx()[ entities_space.connectivity()[ elem_idx ][0] ];
              send_glb_idx_on_rank[p][h] = first_glb_idx;
            }
          }
//          std::cout << std::endl;
//          else    // This check only works with 2 processes, as it MUST be found on the other process
//          {
//            std::cout << PERank << "hash["<<p<<"]["<<h<<"] = " << recv_ghosts_hashed[p][h] << " not found " << std::endl;
//            std::cout << "      possible hashes:" << std::endl;
//            for(hash_to_elem_idx_iter=hash_to_elem_idx.begin(); hash_to_elem_idx_iter!=hash_not_found; ++hash_to_elem_idx_iter)
//            {
//              std::cout << " -- " << hash_to_elem_idx_iter->first << std::endl;
//            }
//          }
        }
      }
    }

    // (6)
    std::vector< std::vector<Uint> > recv_glb_idx_on_rank(Comm::instance().size());
    if (Comm::instance().is_active())
      Comm::instance().all_to_all(send_glb_idx_on_rank,recv_glb_idx_on_rank);
    else
      recv_glb_idx_on_rank[0] = send_glb_idx_on_rank[0];


    for (Uint g=0; g<ghosts.size(); ++g)
    {
      const Space& entities_space = space(*ghosts[g].comp);
      const Uint elem_idx = ghosts[g].idx;
      cf3_assert(elem_idx<entities_space.connectivity().size());
      cf3_assert(elem_idx<entities_space.support().rank().size());

      // 2 cases:
      // (1) rank = UNKNOWN
      // ---> search for the non-UNKNOWN ghost index
      // (2) rank = KNOWN
      // ---> get the ghost idx from the known rank

      const Uint first_loc_idx = entities_space.connectivity()[elem_idx][0];

      const Uint ghost_rank = rank()[first_loc_idx];
      const Uint first_glb_idx = recv_glb_idx_on_rank[ ghost_rank ][g];

      cf3_assert(ghost_rank < Comm::instance().size());
      if (first_glb_idx == UNKNOWN)
        throw ValueNotFound(FromHere(), "Could  not find ghost element "+entities_space.uri().path()+"["+to_str(elem_idx)+"] with hash "+to_str(ghosts_hashed[g])+" on rank "+to_str(ghost_rank));
      for (Uint s=0; s<entities_space.nb_states(); ++s)
      {
        cf3_assert(first_loc_idx+s<glb_idx().size());
        glb_idx()[first_loc_idx+s] = first_glb_idx+s;
        rank()[first_loc_idx+s] = ghost_rank;
      }
    }

    create_coordinates();
  }
}

////////////////////////////////////////////////////////////////////////////////

//common::Table<Uint>::ConstRow Dictionary::indexes_for_element(const Entities& elements, const Uint idx) const
//{
//  Space& space = elements.space(m_space);
//  cf3_assert_desc("space not bound to this dict", &space.fields() == this);
//  return space.connectivity()[idx];
//}

//////////////////////////////////////////////////////////////////////////////////

//common::Table<Uint>::ConstRow Dictionary::indexes_for_element(const Uint unified_idx) const
//{
//  Handle< Component > component;
//  Uint elem_idx;
//  boost::tie(component,elem_idx) = elements_lookup().location(unified_idx);
//  return indexes_for_element(dynamic_cast<Entities&>(*component),elem_idx);
//}

////////////////////////////////////////////////////////////////////////////////

bool Dictionary::has_coordinates() const
{
  return is_not_null(m_coordinates);
}

////////////////////////////////////////////////////////////////////////////////

Field& Dictionary::coordinates()
{
  if (is_null(m_coordinates))
  {
    if (Handle< Field > found = find_component_ptr_with_tag<Field>(*this,mesh::Tags::coordinates()))
    {
      m_coordinates = found;
    }
    else
    {
      throw ValueNotFound(FromHere(),"Dictionary ["+uri().string()+"] has no coordinates field");
    }
  }
  return *m_coordinates;
}

////////////////////////////////////////////////////////////////////////////////

const Field& Dictionary::coordinates() const
{
  if (is_null(m_coordinates))
    throw ValueNotFound(FromHere(),"Dictionary ["+uri().string()+"] has no coordinates field");
  return *m_coordinates;
}

////////////////////////////////////////////////////////////////////////////////

Field& Dictionary::create_coordinates()
{
  if (has_coordinates())
    throw ValueExists(FromHere(),"coordinates cannot be created, they already exist");

  Field& coordinates = create_field("coordinates","coords[vector]");
  boost_foreach(const Handle<Entities>& entities_handle, entities_range())
  {
    Entities& entities = *entities_handle;
    Space& geometry_space = entities.geometry_space();
    const ShapeFunction& geom_sf = geometry_space.shape_function();
    RealMatrix geom_nodes;
    entities.geometry_space().allocate_coordinates(geom_nodes);

    const Space& entities_space = space(entities);
    const ShapeFunction& sf = entities_space.shape_function();
    const RealMatrix& local_coords = sf.local_coordinates();

    RealMatrix interpolation(sf.nb_nodes(),geom_sf.nb_nodes());
    for (Uint i=0; i<sf.nb_nodes(); ++i)
      interpolation.row(i) = geom_sf.value(local_coords.row(i));

    RealMatrix coords(sf.nb_nodes(),geom_nodes.cols());

    for (Uint e=0; e<entities.size(); ++e)
    {
      entities.geometry_space().put_coordinates(geom_nodes,e);
      coords = interpolation*geom_nodes;

      for (Uint i=0; i<sf.nb_nodes(); ++i)
      {
        const Uint pt = entities_space.connectivity()[e][i];
        for(Uint d=0; d<coords.cols(); ++d)
          coordinates[pt][d] = coords(i,d);
      }
    }
  }

  m_coordinates = Handle<Field>(coordinates.handle<Component>());
  return coordinates;
}

////////////////////////////////////////////////////////////////////////////////

DynTable<Uint>& Dictionary::glb_elem_connectivity()
{
  if (is_null(m_glb_elem_connectivity))
  {
    m_glb_elem_connectivity = create_static_component< DynTable<Uint> >("glb_elem_connectivity");
    m_glb_elem_connectivity->add_tag("glb_elem_connectivity");
    m_glb_elem_connectivity->resize(size());
  }
  return *m_glb_elem_connectivity;
}



////////////////////////////////////////////////////////////////////////////////

void Dictionary::signature_create_field( SignalArgs& node )
{
  SignalOptions options( node );

  options.add_option<std::string>("name")
      .description("Name of the field" );

  options.add_option<std::string>("variables")
      .description("Variables description of the field" );

}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::signal_create_field( SignalArgs& node )
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
