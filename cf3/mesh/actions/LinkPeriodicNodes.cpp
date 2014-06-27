// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/List.hpp"
#include "common/Option.hpp"
#include "common/OptionList.hpp"

#include "mesh/ConnectivityData.hpp"
#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshAdaptor.hpp"
#include "mesh/Field.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ElementData.hpp"

#include "LinkPeriodicNodes.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

namespace detail {

/// Check if two points are close to each other
inline bool is_close(const RealVector& a, const RealVector& b)
{
  return (b-a).squaredNorm() < 1e-8;
}

// Get the used nodes list of a region, including any nodes that live on the current CPU
// but are only linked to on other CPUs
std::vector<Uint> used_nodes(const mesh::Region& region, const mesh::Dictionary& dict)
{
  // Start with a used node list of the current CPU
  boost::shared_ptr< common::List< Uint > > own_used_node_list = build_used_nodes_list(region, dict, false, false);
  std::vector<Uint> used_node_vec;
  common::PE::Comm& comm = common::PE::Comm::instance();
  
  if(!comm.is_active())
  {
    used_node_vec.assign(own_used_node_list->array().begin(), own_used_node_list->array().end());
    return used_node_vec;
  }
  
  std::vector<Uint> own_gids; own_gids.reserve(own_used_node_list->size());
  const Uint nb_nodes = dict.size();
  std::vector<bool> is_added(nb_nodes, false);
  BOOST_FOREACH(const Uint own_idx, own_used_node_list->array())
  {
    own_gids.push_back(dict.glb_idx()[own_idx]);
    is_added[own_idx] = true; // All local nodes are in the list automatically
  }
  
  std::vector< std::vector<Uint> > recv_gids;
  comm.all_gather(own_gids, recv_gids);
  
  std::set<Uint> global_boundary_gids; // GIDs that reside on other CPUs
  const Uint nb_procs = comm.size();
  for(Uint i = 0; i != nb_procs; ++i)
  {
    if(i == comm.rank())
      continue;
    
    BOOST_FOREACH(const Uint gid, recv_gids[i])
    {
      global_boundary_gids.insert(gid);
    }
  }
  
  std::list<Uint> extra_nodes;
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    if(is_added[i])
      continue;
    
    if(global_boundary_gids.count(dict.glb_idx()[i]))
    {
      is_added[i] = true;
      extra_nodes.push_back(i);
    }
  }
  
  used_node_vec.reserve(own_used_node_list->size()+ extra_nodes.size());
  used_node_vec.insert(used_node_vec.end(), own_used_node_list->array().begin(), own_used_node_list->array().end());
  used_node_vec.insert(used_node_vec.end(), extra_nodes.begin(), extra_nodes.end());
  return used_node_vec;
}

}

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < LinkPeriodicNodes, MeshTransformer, mesh::actions::LibActions> LinkPeriodicNodes_Builder;

////////////////////////////////////////////////////////////////////////////////

LinkPeriodicNodes::LinkPeriodicNodes(const std::string& name) : MeshTransformer(name)
{
  options().add("source_region", m_source_region)
      .pretty_name("Source Region")
      .description("Region in which the nodes will be replaced by there periodic counterparts")
      .link_to(&m_source_region)
      .mark_basic();


  options().add("destination_region", m_destination_region)
      .pretty_name("Destination Region")
      .description("Region containing the periodic replacements for the source region")
      .link_to(&m_destination_region)
      .mark_basic();

  options().add("translation_vector", m_translation_vector)
      .pretty_name("Translation Vector")
      .description("Vector over which the source and destination nodes are translated")
      .link_to(&m_translation_vector)
      .mark_basic();
}

void LinkPeriodicNodes::execute()
{
  Mesh& mesh = *m_mesh;
  const Field& coords = mesh.geometry_fields().coordinates();
  common::PE::Comm& comm = common::PE::Comm::instance();

  Handle< common::List<Uint> > periodic_links_nodes_h(mesh.geometry_fields().get_child("periodic_links_nodes"));
  Handle< common::List<bool> > periodic_links_active_h(mesh.geometry_fields().get_child("periodic_links_active"));

  if(is_null(periodic_links_nodes_h))
  {
    cf3_assert(is_null(periodic_links_active_h));
    periodic_links_nodes_h = mesh.geometry_fields().create_component< common::List<Uint> >("periodic_links_nodes");
    periodic_links_active_h = mesh.geometry_fields().create_component< common::List<bool> >("periodic_links_active");
    periodic_links_nodes_h->resize(mesh.geometry_fields().size());
    periodic_links_active_h->resize(mesh.geometry_fields().size());
  }

  common::List<Uint>& periodic_links_nodes = *periodic_links_nodes_h;
  common::List<bool>& periodic_links_active = *periodic_links_active_h;
  cf3_assert(periodic_links_nodes.size() == mesh.geometry_fields().size());
  cf3_assert(periodic_links_active.size() == mesh.geometry_fields().size());

  const std::vector<Uint> source_nodes = detail::used_nodes(*m_source_region, mesh.geometry_fields());
  const std::vector<Uint> destination_nodes = detail::used_nodes(*m_destination_region, mesh.geometry_fields());

  CFdebug << "Linking source region " << m_source_region->uri().string() << " to destination region " << m_destination_region->uri().string() << CFendl;

 if(source_nodes.size() != destination_nodes.size())
   throw common::SetupError(FromHere(), "Source and destination regions do not have the same number of nodes: " + m_source_region->name() + " has " + common::to_str(source_nodes.size()) + " nodes, " + m_destination_region->name() + " has " + common::to_str(destination_nodes.size()) + " nodes");

  if(m_translation_vector.size() != mesh.dimension())
    throw common::SetupError(FromHere(), "Translation vector number of components does not match mesh dimension");

  const RealVector translation_vector = to_vector(m_translation_vector);

  BOOST_FOREACH(const Uint source_node_idx, source_nodes)
  {
    const RealVector source_coord = to_vector(coords[source_node_idx]) + translation_vector;
    BOOST_FOREACH(const Uint dest_node_idx, destination_nodes)
    {
      if(detail::is_close(source_coord, to_vector(coords[dest_node_idx])))
      {
        periodic_links_active[source_node_idx] = true;
        periodic_links_nodes[source_node_idx] = dest_node_idx;
      }
    }
  }

  boost::shared_ptr<NodeConnectivity> node_connectivity = common::allocate_component<NodeConnectivity>("node_connectivity");
  node_connectivity->initialize(common::find_components_recursively_with_filter<mesh::Entities>(*m_destination_region, IsElementsSurface()));

  BOOST_FOREACH(mesh::Entities& elements, common::find_components_recursively_with_filter<mesh::Entities>(*m_source_region, IsElementsSurface()))
  {
    Handle< common::List<Uint> > periodic_links_elements_h(elements.get_child("periodic_links_elements"));
    if(is_null(periodic_links_elements_h))
      periodic_links_elements_h = elements.create_component< common::List<Uint> >("periodic_links_elements");

    common::List<Uint>& periodic_links_elements = *periodic_links_elements_h;

    // This is a link to the component that holds the linked elements
    Handle< common::Link > periodic_link(periodic_links_elements.get_child("periodic_link"));
    if(is_null(periodic_link))
    {
      periodic_link = periodic_links_elements.create_component<common::Link>("periodic_link");
    }

    Handle<Elements const> elements_to_link;

    const Uint nb_elements = elements.size();
    periodic_links_elements.resize(nb_elements);
    const Connectivity& connectivity = elements.geometry_space().connectivity();
    cf3_assert(connectivity.size() == nb_elements);
    const Uint nb_element_nodes = connectivity.row_size();
    std::vector<Uint> translated_row(nb_element_nodes);
    for(Uint elem_idx = 0; elem_idx != nb_elements; ++elem_idx)
    {
      const Connectivity::ConstRow row = connectivity[elem_idx];
      for(Uint i = 0; i != nb_element_nodes; ++i)
      {
        const Uint element_node = row[i];
        translated_row[i] = periodic_links_nodes[element_node];

        if(!periodic_links_active[element_node])
          throw common::SetupError(FromHere(), "Error: node " + common::to_str(element_node) + " from region " + elements.uri().path() + " has no periodic link");
      }
      std::sort(translated_row.begin(), translated_row.end());
      bool found_match = false;
      BOOST_FOREACH(const NodeConnectivity::ElementReferenceT elref, node_connectivity->node_element_range(translated_row.front()))
      {
        const Entities& other_elements = *node_connectivity->entities()[elref.first];
        const Uint other_idx = elref.second;
        const Connectivity& other_conn = other_elements.geometry_space().connectivity();
        const Connectivity::ConstRow other_row = other_conn[other_idx];
        std::vector<Uint> other_row_sorted(other_row.begin(), other_row.end());
        std::sort(other_row_sorted.begin(), other_row_sorted.end());
        cf3_assert(other_row_sorted.size() == translated_row.size());
        if(other_row_sorted == translated_row)
        {
          found_match = true;
          Handle<Elements const> other_elements_h(other_elements.handle());
          if(is_null(elements_to_link))
          {
            elements_to_link = other_elements_h;
          }
          else if(elements_to_link != other_elements_h)
          {
            throw common::SetupError(FromHere(), "Periodic links spread across elements collections!");
          }
          periodic_links_elements[elem_idx] = other_idx;
          break;
        }
      }
      if(!found_match)
        throw common::SetupError(FromHere(), "No periodic match found for element " + common::to_str(elem_idx) + " of elements " + elements.uri().path());
    }
    // Set same linked elements across CPUs, even if no elements are found on the current rank
    if(comm.is_active())
    {
      int linked_elem_idx = is_null(elements_to_link) ? -1 : elements_to_link->entities_idx();
      std::vector<int> recv;
      comm.all_gather(linked_elem_idx, recv);
      BOOST_FOREACH(const int new_idx, recv)
      {
        if(new_idx != -1)
        {
          if(linked_elem_idx != -1)
          {
            cf3_always_assert(new_idx == linked_elem_idx);
          }
          else
          {
            linked_elem_idx = new_idx;
          }
        }
      }
      if(is_null(elements_to_link))
      {
        elements_to_link = Handle<Elements>(mesh.elements()[linked_elem_idx]);
      }
    }

    cf3_assert(is_not_null(elements_to_link));
    periodic_link->link_to(const_cast<Elements&>(*elements_to_link));
    cf3_always_assert(nb_elements == elements_to_link->size());
  }
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
