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
#include "Common/Tags.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Common/PE/Comm.hpp"
#include "Common/PE/CommPattern.hpp"

#include "Math/VariablesDescriptor.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/FieldGroup.hpp"
#include "Mesh/Field.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CUnifiedData.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CFaces.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CConnectivity.hpp"

#include "Math/Consts.hpp"
#define UNKNOWN Math::Consts::uint_max()

namespace CF {
namespace Mesh {

using namespace boost::assign;

using namespace Common;
using namespace Common::PE;

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

CommPattern& FieldGroup::comm_pattern()
{
  if(m_comm_pattern.expired())
  {
    PE::CommPattern& comm_pattern = create_component<PE::CommPattern>("CommPattern");

    comm_pattern.insert("gid",glb_idx().array(),false);
    comm_pattern.setup(comm_pattern.get_child("gid").as_ptr<PE::CommWrapper>(),rank().array());

    m_comm_pattern = comm_pattern.as_ptr<Common::PE::CommPattern>();
  }

  return *m_comm_pattern.lock();
}

//////////////////////////////////////////////////////////////////////////////

bool FieldGroup::is_ghost(const Uint idx) const
{
  cf_assert_desc(to_str(idx)+">="+to_str(size()),idx < size());
  cf_assert(size() == m_rank->size());
  cf_assert(idx<m_rank->size());
  return (*m_rank)[idx] != Comm::instance().rank();
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
}

////////////////////////////////////////////////////////////////////////////////

Field& FieldGroup::create_field(const std::string &name, Math::VariablesDescriptor& variables_descriptor)
{
  Field& field = create_component<Field>(name);
  field.set_field_group(*this);
  field.set_topology(topology());
  field.set_basis(m_basis);
  field.set_descriptor(variables_descriptor);
  if (variables_descriptor.option(Common::Tags::dimension()).value<Uint>() == 0)
    field.descriptor().configure_option(Common::Tags::dimension(),parent().as_type<CMesh>().dimension());
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
      boost_foreach(CEntities& faces, find_components_recursively_with_tag<CEntities>(topology(),Mesh::Tags::face_entity()))
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

  if (m_space != Mesh::Tags::geometry())
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

// copy of CMeshPartitioner.cpp::flex_all_gather
template <typename T>
void hash_all_gather(const std::vector<T>& send, std::vector<std::vector<T> >& recv)
{
  std::vector<int> strides;
  PE::Comm::instance().all_gather((int)send.size(),strides);
  std::vector<int> displs(strides.size());
  if (strides.size())
  {
    int sum_strides = strides[0];
    displs[0] = 0;
    for (Uint i=1; i<strides.size(); ++i)
    {
      displs[i] = displs[i-1] + strides[i-1];
      sum_strides += strides[i];
    }
    std::vector<T> recv_linear(sum_strides);
    MPI_CHECK_RESULT(MPI_Allgatherv, ((void*)&send[0], (int)send.size(), get_mpi_datatype<T>(), &recv_linear[0], &strides[0], &displs[0], get_mpi_datatype<T>(), PE::Comm::instance().communicator()));
    recv.resize(strides.size());
    for (Uint i=0; i<strides.size(); ++i)
    {
      recv[i].resize(strides[i]);
      for (Uint j=0; j<strides[i]; ++j)
      {
        recv[i][j]=recv_linear[displs[i]+j];
      }
    }
  }
  else
  {
    recv.resize(0);
  }
}

// copy of CMeshPartitioner.cpp::flex_all_to_all
template <typename T>
void hash_all_to_all(const std::vector<std::vector<T> >& send, std::vector<std::vector<T> >& recv)
{
  std::vector<int> send_strides(send.size());
  std::vector<int> send_displs(send.size());
  for (Uint i=0; i<send.size(); ++i)
    send_strides[i] = send[i].size();

  send_displs[0] = 0;
  for (Uint i=1; i<send.size(); ++i)
    send_displs[i] = send_displs[i-1] + send_strides[i-1];

  std::vector<T> send_linear(send_displs.back()+send_strides.back());
  for (Uint i=0; i<send.size(); ++i)
    for (Uint j=0; j<send[i].size(); ++j)
      send_linear[send_displs[i]+j] = send[i][j];

  std::vector<int> recv_strides(PE::Comm::instance().size());
  std::vector<int> recv_displs(PE::Comm::instance().size());
  PE::Comm::instance().all_to_all(send_strides,recv_strides);
  recv_displs[0] = 0;
  for (Uint i=1; i<PE::Comm::instance().size(); ++i)
    recv_displs[i] = recv_displs[i-1] + recv_strides[i-1];

  std::vector<T> recv_linear(recv_displs.back()+recv_strides.back());
  MPI_CHECK_RESULT(MPI_Alltoallv, (&send_linear[0], &send_strides[0], &send_displs[0], PE::get_mpi_datatype<Uint>(), &recv_linear[0], &recv_strides[0], &recv_displs[0], get_mpi_datatype<Uint>(), PE::Comm::instance().communicator()));

  recv.resize(recv_strides.size());
  for (Uint i=0; i<recv_strides.size(); ++i)
  {
    recv[i].resize(recv_strides[i]);
    for (Uint j=0; j<recv_strides[i]; ++j)
    {
      recv[i][j]=recv_linear[recv_displs[i]+j];
    }
  }
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

    // step 3: resize
    // --------------
    resize(points.size());
    m_coordinates = coordinates.as_ptr<Field>();
    for (Uint i=0; i<size(); ++i)
      rank()[i] = UNKNOWN;

    // step 2: collect nodes in a set
    // ------------------------------
    boost_foreach(CEntities& entities, entities_range())
    {
      Geometry& geometry = entities.geometry();
      CConnectivity& geometry_node_connectivity = entities.geometry_space().connectivity();
      CList<Uint>& geometry_rank = entities.geometry().rank();
      entities.space(m_space).get_child("bound_fields").as_type<CLink>().link_to(*this);
      const ShapeFunction& shape_function = entities.space(m_space).shape_function();
      CConnectivity& connectivity = entities.space(m_space).connectivity();
      connectivity.set_row_size(shape_function.nb_nodes());
      connectivity.resize(entities.size());
      for (Uint elem=0; elem<entities.size(); ++elem)
      {
        Uint nodes_rank = UNKNOWN;
        std::set<Uint> possible_ranks;
        boost_foreach(const Uint geometry_node, geometry_node_connectivity[elem])
          possible_ranks.insert(geometry_rank[geometry_node]);

        if (possible_ranks.size() == 1)
          nodes_rank = *possible_ranks.begin();

        elem_coordinates = entities.get_coordinates(elem);
        for (Uint node=0; node<shape_function.nb_nodes(); ++node)
        {
          RealVector space_coordinates = entities.element_type().shape_function().value(shape_function.local_coordinates().row(node)) * elem_coordinates ;
          std::size_t hash = hash_value(space_coordinates);
          Uint idx = std::distance(points.begin(), points.find(hash));
          connectivity[elem][node] = idx;
          coordinates.set_row(idx, space_coordinates);

          /// @todo find way to set ranks of nodes of cells where everything is owned
          // rank()[idx] = std::min(rank()[idx],nodes_rank);
        }
      }
    }

    // step 4: add lookup to connectivity tables
    // -----------------------------------------
    boost_foreach(CEntities& entities, entities_range())
        entities.space(m_space).connectivity().create_lookup().add(*this);

    // step 5: fix unknown ranks
    // -------------------------
    cf_assert(size() == m_rank->size());
    cf_assert(size() == m_glb_idx->size());
    cf_assert(size() == m_coordinates->size());

    std::deque<Uint> unknown_ranks;
    std::map<size_t,Uint> hash_to_idx;
    std::map<size_t,Uint>::iterator hash_to_idx_iter;
    std::map<size_t,Uint>::iterator hash_not_found = hash_to_idx.end();

    // - Identify unknown ranks, give glb_idx to unknown ranks
    for (Uint i=0; i<size(); ++i)
    {
      if ( rank()[i] == UNKNOWN)
        unknown_ranks.push_back(i);
    }
    std::vector<size_t> coord_hash_with_unknown_rank(unknown_ranks.size());
    RealVector dummy(coordinates.row_size());
    Uint c(0);
    for (Uint i=0; i<size(); ++i)
    {
      for (Uint d=0; d<coordinates.row_size(); ++d)
        dummy[d] = coordinates[i][d];
      size_t hash = hash_value(dummy);
      hash_to_idx[hash] = i;
      if ( rank()[i] == UNKNOWN)
        coord_hash_with_unknown_rank[c++]=hash;
    }

    // - Communicate unknown ranks to all processes
    std::vector< std::vector<size_t> > recv_hash;
    hash_all_gather(coord_hash_with_unknown_rank,recv_hash);

    // - Search this process contains the missing ranks of other processes
    std::vector< std::vector<Uint> > send_found_on_rank(Comm::instance().size());
    for (Uint p=0; p<Comm::instance().size(); ++p)
    {
      send_found_on_rank[p].resize(recv_hash[p].size(),false);
      {
        for (Uint h=0; h<recv_hash[p].size(); ++h)
        {
          hash_to_idx_iter = hash_to_idx.find(recv_hash[p][h]);
          if ( hash_to_idx_iter != hash_not_found )
          {
            send_found_on_rank[p][h] = true;
          }
        }
      }

    }

    // - Communicate which processes found the missing ranks
    std::vector< std::vector<Uint> > recv_found_on_rank(Comm::instance().size());
    hash_all_to_all(send_found_on_rank,recv_found_on_rank);

    // - Set the missing rank to the lowest process number that found it
    for (Uint h=0; h<coord_hash_with_unknown_rank.size(); ++h)
    {
      Uint rank_that_owns = UNKNOWN;

      for (Uint p=0; p<Comm::instance().size(); ++p)
      {
        if (recv_found_on_rank[p][h])
          rank_that_owns = std::min( rank_that_owns, p);
      }
      rank()[unknown_ranks[h]] = rank_that_owns;
      if (rank_that_owns != Comm::instance().rank())
      {
        if (is_ghost(unknown_ranks[h]) == false)
          throw BadValue(FromHere(),"");
      }
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

    std::vector< std::vector<size_t> > recv_ghosts_hashed;
    hash_all_gather(ghosts_hashed,recv_ghosts_hashed);

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
    hash_all_to_all(send_glb_idx_on_rank,recv_glb_idx_on_rank);

    // - Set the missing rank to the lowest process number that found it
    for (Uint g=0; g<ghosts.size(); ++g)
    {
      cf_assert(rank()[ghosts[g]] < Comm::instance().size());
      glb_idx()[ghosts[g]] = recv_glb_idx_on_rank[rank()[ghosts[g]]][g];
    }
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
    {
      entities.space(m_space).connectivity().create_lookup().add(*this);
      CSpace& space = entities.space(m_space);
      for (Uint e=0; e<entities.size(); ++e)
      {
        CConnectivity::ConstRow field_indexes = space.indexes_for_element(e);
        boost_foreach(const Uint idx, field_indexes)
          rank()[idx] = entities.rank()[e];
      }
    }
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

bool FieldGroup::has_coordinates() const
{
  return is_not_null(m_coordinates);
}

////////////////////////////////////////////////////////////////////////////////

Field& FieldGroup::coordinates() const
{
  if (is_null(m_coordinates))
    throw ValueNotFound(FromHere(),"FieldGroup ["+uri().string()+"] has no coordinates field");

  return *m_coordinates;
}

////////////////////////////////////////////////////////////////////////////////

Field& FieldGroup::create_coordinates()
{
  if (has_coordinates())
    throw ValueExists(FromHere(),"coordinates cannot be created, they already exist");

  Field& coordinates = create_field("coordinates","coords[vector]");
  boost_foreach(CEntities& entities, entities_range())
  {
    CSpace& geometry_space = entities.geometry_space();
    const ShapeFunction& geom_sf = geometry_space.shape_function();
    RealMatrix geom_nodes;
    entities.allocate_coordinates(geom_nodes);

    CSpace& space = entities.space(m_space);
    const ShapeFunction& sf = space.shape_function();
    const RealMatrix& local_coords = sf.local_coordinates();

    RealMatrix interpolation(sf.nb_nodes(),geom_sf.nb_nodes());
    for (Uint i=0; i<sf.nb_nodes(); ++i)
      interpolation.row(i) = geom_sf.value(local_coords.row(i));

    RealMatrix coords(sf.nb_nodes(),geom_nodes.cols());

    for (Uint e=0; e<entities.size(); ++e)
    {
      CConnectivity::ConstRow field_index = space.indexes_for_element(e);

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

} // Mesh
} // CF

#undef UNKNOWN
