// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/tokenizer.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/StreamHelpers.hpp"
#include "common/StringConversion.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "mesh/actions/MergeMeshes.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/FaceCellConnectivity.hpp"
#include "mesh/NodeElementConnectivity.hpp"
#include "mesh/Node2FaceCellConnectivity.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Connectivity.hpp"

#include "math/Functions.hpp"
#include "math/Consts.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;
  using namespace math::Functions;
  using namespace math::Consts;

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

boost::shared_ptr<Mesh> MergeMeshes::merge(const std::vector<Handle<const Mesh> > &meshes)
{

  boost::shared_ptr<Mesh> merged = allocate_component<Mesh>("merged_mesh");

//  CFinfo << merged->tree() << CFendl;

  // 1) Initialize nodes
  Uint total_nb_nodes=0;
  Uint dimension=0;
  boost_foreach( const Handle<Mesh const>& mesh, meshes)
  {
    total_nb_nodes += mesh->geometry_fields().size();
    dimension = std::max(mesh->dimension(),dimension);
  }
  merged->initialize_nodes(total_nb_nodes,dimension);


  // counter that will be updated at end of each mesh addition
  Uint previous_nb_nodes=0;

  boost_foreach( const Handle<Mesh const>& mesh, meshes)
  {
    // 2) add all nodes
    for (Uint n=0; n<mesh->geometry_fields().size(); ++n)
    {
      merged->geometry_fields().coordinates()[previous_nb_nodes+n] = mesh->geometry_fields().coordinates()[n];
      merged->geometry_fields().glb_idx()[previous_nb_nodes+n] = mesh->geometry_fields().glb_idx()[n];
      merged->geometry_fields().rank()[previous_nb_nodes+n] = mesh->geometry_fields().rank()[n];
    }

    // 3) add all regions and elements
    boost_foreach(const Region& mesh_region, find_components_recursively<Region>(mesh->topology()))
    {
      std::string mesh_region_relative_path = mesh_region.uri().path();
      boost::algorithm::replace_first(mesh_region_relative_path,mesh->topology().uri().path()+"/","");

      std::vector<std::string> vec;
      typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
      boost::char_separator<char> sep("/");
      Tokenizer tokens(mesh_region_relative_path, sep);
      for (Tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
        vec.push_back(*tok_iter);

      Handle<Component> found;


      Handle<Region> merged_region = merged->topology().handle<Region>();
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
          merged_entities = merged_region->create_elements(mesh_region_entities.element_type().derived_type_name(),merged->geometry_fields()).handle<Entities>();
        CFinfo << "add elements of " << mesh_region_entities.uri() << CFendl;

        Connectivity::Buffer merged_connectivity = merged_entities->geometry_space().connectivity().create_buffer();
        List<Uint>::Buffer merged_rank = merged_entities->rank().create_buffer();
        List<Uint>::Buffer merged_glb_idx = merged_entities->glb_idx().create_buffer();

        Connectivity& mesh_connectivity = mesh_region_entities.geometry_space().connectivity();
        std::vector<Uint> merged_connectivity_row(merged_entities->geometry_space().shape_function().nb_nodes());

        const Uint nb_elem = mesh_region_entities.size();
        for (Uint e=0; e<nb_elem; ++e)
        {
          for (Uint n=0; n<merged_connectivity_row.size(); ++n)
            merged_connectivity_row[n] = previous_nb_nodes + mesh_connectivity[e][n];
          merged_connectivity.add_row(merged_connectivity_row);

          merged_rank.add_row(mesh_region_entities.rank()[e]);
          merged_glb_idx.add_row(mesh_region_entities.glb_idx()[e]);
        }
      }
    }

    previous_nb_nodes += mesh->geometry_fields().size();
  }

  CFinfo << merged->tree() << CFendl;

  return merged;
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
