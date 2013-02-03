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

  options().add("link_ghosts", true)
      .pretty_name("Link Ghosts")
      .description("Create links for ghost nodes in the source region")
      .mark_basic();
}

void LinkPeriodicNodes::execute()
{
  const bool link_ghosts = options().value<bool>("link_ghosts");

  Mesh& mesh = *m_mesh;
  const Field& coords = mesh.geometry_fields().coordinates();

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

  boost::shared_ptr< common::List< Uint > > source_nodes = build_used_nodes_list(*m_source_region, mesh.geometry_fields(), true, false);
  boost::shared_ptr< common::List< Uint > > destination_nodes = build_used_nodes_list(*m_destination_region, mesh.geometry_fields(), true, false);
  
  CFdebug << "Linking source region " << m_source_region->uri().string() << " to destination region " << m_destination_region->uri().string() << CFendl;

  if(source_nodes->size() != destination_nodes->size())
    throw common::SetupError(FromHere(), "Source and destination regions do not have the same number of nodes");

  if(m_translation_vector.size() != mesh.dimension())
    throw common::SetupError(FromHere(), "Translation vector number of components does not match mesh dimension");

  const RealVector translation_vector = to_vector(m_translation_vector);

  bool matched_region = true;
  
  BOOST_FOREACH(const Uint source_node_idx, source_nodes->array())
  {
    if(periodic_links_active[source_node_idx])
      continue;
    if(!link_ghosts && mesh.geometry_fields().is_ghost(source_node_idx))
      continue;

    bool found_match = false;
    const RealVector source_coord = to_vector(coords[source_node_idx]) + translation_vector;
    BOOST_FOREACH(const Uint dest_node_idx, destination_nodes->array())
    {
      if(detail::is_close(source_coord, to_vector(coords[dest_node_idx])))
      {
        periodic_links_active[source_node_idx] = true;
        periodic_links_nodes[source_node_idx] = dest_node_idx;
        found_match = true;
      }
    }
    if(!found_match)
    {
      matched_region = false;
      break;
    }
  }
  
  if(!matched_region)
  {
    RealVector source_centroid(coords.row_size());
    source_centroid.setZero();
    BOOST_FOREACH(const Uint source_node_idx, source_nodes->array())
    {
      source_centroid += to_vector(coords[source_node_idx]);
    }
    source_centroid /= source_nodes->size();
    
    RealVector dest_centroid(coords.row_size());
    dest_centroid.setZero();
    BOOST_FOREACH(const Uint dest_node_idx, destination_nodes->array())
    {
      dest_centroid += to_vector(coords[dest_node_idx]);
    }
    dest_centroid /= destination_nodes->size();
    
    std::stringstream errstr;
    errstr << "source and destination boundaries do not match. Centroid offset vector is " << (dest_centroid - source_centroid).transpose();
    throw common::SetupError(FromHere(), errstr.str());
  }

}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
