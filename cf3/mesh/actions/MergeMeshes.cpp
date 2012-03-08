// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/algorithm/string/replace.hpp>
#include <boost/tokenizer.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Map.hpp"

#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Connectivity.hpp"

#include "mesh/actions/MergeMeshes.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < MergeMeshes, Component, mesh::actions::LibActions> MergeMeshes_Builder;

//////////////////////////////////////////////////////////////////////////////

MergeMeshes::MergeMeshes( const std::string& name )
: Component(name)
{

  properties()["brief"] = std::string("Merges meshes into a new mesh");
  std::string desc;
  properties()["description"] = desc;

  options().add_option("meshes", std::vector<URI>())
    .description("Meshes to merge");
}

/////////////////////////////////////////////////////////////////////////////

void MergeMeshes::merge_mesh(const Mesh& mesh, Mesh& merged)
{
//  CFinfo << "merging mesh " << mesh.uri() << CFendl;
  // 1) Initialize nodes
  Uint total_nb_nodes=merged.geometry_fields().size();
  Uint node=total_nb_nodes;
  Uint dimension=merged.dimension();
  total_nb_nodes += mesh.geometry_fields().size();
  dimension = std::max(mesh.dimension(),dimension);

  merged.initialize_nodes(total_nb_nodes,dimension);

  Handle< Component > found_nodes_glb_2_loc = merged.geometry_fields().get_child("glb_to_loc");
  Handle< Map<Uint,Uint> > nodes_glb_2_loc_handle;
  if (found_nodes_glb_2_loc)
    nodes_glb_2_loc_handle = found_nodes_glb_2_loc->handle< Map<Uint,Uint> >();
  else
    nodes_glb_2_loc_handle = merged.geometry_fields().create_component< Map<Uint,Uint> >("glb_to_loc");
  Map<Uint,Uint>& nodes_glb_2_loc = *nodes_glb_2_loc_handle;

  // 2) add all nodes
  for (Uint n=0; n<mesh.geometry_fields().size(); ++n)
  {
    if (nodes_glb_2_loc.find(mesh.geometry_fields().glb_idx()[n]) == nodes_glb_2_loc.end())
    {
      merged.geometry_fields().coordinates()[node] = mesh.geometry_fields().coordinates()[n];
      merged.geometry_fields().glb_idx()[node] = mesh.geometry_fields().glb_idx()[n];
      merged.geometry_fields().rank()[node] = mesh.geometry_fields().rank()[n];
      nodes_glb_2_loc.push_back(mesh.geometry_fields().glb_idx()[n],node);
      ++node;
    }
    else
    {
      Uint loc_idx = nodes_glb_2_loc[mesh.geometry_fields().glb_idx()[n]];
      for (Uint d=0; d<dimension; ++d)
      {
        if (merged.geometry_fields().coordinates()[loc_idx][d] != mesh.geometry_fields().coordinates()[n][d])
          throw NotSupported(FromHere(),"Only meshes that share the same global indices are supported");
      }
    }
  }

  // 3) add all regions and elements
  boost_foreach(const Region& mesh_region, find_components_recursively<Region>(mesh.topology()))
  {
    std::string mesh_region_relative_path = mesh_region.uri().path();
    boost::algorithm::replace_first(mesh_region_relative_path,mesh.topology().uri().path()+"/","");

    std::vector<std::string> vec;
    typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
    boost::char_separator<char> sep("/");
    Tokenizer tokens(mesh_region_relative_path, sep);
    for (Tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
      vec.push_back(*tok_iter);

    Handle<Component> found;


    Handle<Region> merged_region = merged.topology().handle<Region>();
    boost_foreach(const std::string& path, vec)
    {
      found = merged_region->access_component(path);
      if ( found )
        merged_region = found->handle<Region>();
      else
        merged_region = merged_region->create_component<Region>(path);
    }

    Handle<Entities> merged_entities;
    boost_foreach(const Entities& mesh_region_entities, find_components<Entities>(mesh_region))
    {
      found = merged_region->get_child(mesh_region_entities.name());
      if (found)
        merged_entities = found->handle<Entities>();
      else
        merged_entities = merged_region->create_elements(mesh_region_entities.element_type().derived_type_name(),merged.geometry_fields()).handle<Entities>();

      Connectivity::Buffer merged_connectivity = merged_entities->geometry_space().connectivity().create_buffer();
      List<Uint>::Buffer merged_rank = merged_entities->rank().create_buffer();
      List<Uint>::Buffer merged_glb_idx = merged_entities->glb_idx().create_buffer();

      Connectivity& mesh_connectivity = mesh_region_entities.geometry_space().connectivity();
      std::vector<Uint> merged_connectivity_row(merged_entities->geometry_space().shape_function().nb_nodes());

      const Uint nb_elem = mesh_region_entities.size();
      for (Uint e=0; e<nb_elem; ++e)
      {
        for (Uint n=0; n<merged_connectivity_row.size(); ++n)
          merged_connectivity_row[n] = nodes_glb_2_loc[mesh.geometry_fields().glb_idx()[mesh_connectivity[e][n]]];
        merged_connectivity.add_row(merged_connectivity_row);

        merged_rank.add_row(mesh_region_entities.rank()[e]);
        merged_glb_idx.add_row(mesh_region_entities.glb_idx()[e]);
      }
    }
  }
  merged.geometry_fields().resize(node);

  // The problem is that there are no ghost nodes defined.
  // After merging all meshes, a call to "fix_ranks" must be done.

  merged.update_statistics();
  merged.check_sanity();

}

//////////////////////////////////////////////////////////////////////////////

void MergeMeshes::fix_ranks(Mesh& merged)
{
//  CFinfo << "fixing ranks" << CFendl;
  ///so recompute ranks
  // Lower cpu's hold all nodes, higher cpu's try to see if lower cpu's have
  // the same node

  Handle< Component > found_nodes_glb_2_loc = merged.geometry_fields().get_child("glb_to_loc");
  Handle< Map<Uint,Uint> > nodes_glb_2_loc_handle;
  if (found_nodes_glb_2_loc)
    nodes_glb_2_loc_handle = found_nodes_glb_2_loc->handle< Map<Uint,Uint> >();
  else
    nodes_glb_2_loc_handle = merged.geometry_fields().create_component< Map<Uint,Uint> >("glb_to_loc");
  Map<Uint,Uint>& nodes_glb_2_loc = *nodes_glb_2_loc_handle;


  // nodes to send to other cpu's
  std::vector<Uint> glb_nodes(merged.geometry_fields().size());
  for (Uint n=0; n<glb_nodes.size(); ++n)
  {
    // fill the nodes to send to other cpu's
    glb_nodes[n] = merged.geometry_fields().glb_idx()[n];

    // assign the rank to be ourself to start with. The rank can only become lower.
    merged.geometry_fields().rank()[n] = PE::Comm::instance().rank();
  }

  // Every cpu broadcasts to cpu's with higher rank
  for (Uint root=0; root<PE::Comm::instance().size(); ++root)
  {
//    CFinfo << "broadcasting from root " << root << CFendl;
    PE::Comm::instance().barrier();

    std::vector<Uint> glb_nodes_from_root(0);
    PE::Comm::instance().broadcast(glb_nodes,glb_nodes_from_root,root);

    // cpu's with higher rank will adapt the ranks of the nodes
    if (PE::Comm::instance().rank() > root)
    {

      boost_foreach(const Uint glb_node, glb_nodes_from_root)
      {
        if (nodes_glb_2_loc.find(glb_node) != nodes_glb_2_loc.end())
        {
          Uint loc_node = nodes_glb_2_loc[glb_node];

          // if the rank is yourself, it means that the glb_node has not been found
          // before. Change then the rank to the broacasting cpu.
          if (merged.geometry_fields().rank()[loc_node] == PE::Comm::instance().rank())
          {
            merged.geometry_fields().rank()[loc_node] = root;
          }
        }
      }

    }

  }

}


//////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3
