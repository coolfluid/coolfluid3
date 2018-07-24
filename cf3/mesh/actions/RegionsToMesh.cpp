// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"

#include "common/PE/Comm.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ElementData.hpp"

#include "RegionsToMesh.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < RegionsToMesh, MeshTransformer, mesh::actions::LibActions> RegionsToMesh_Builder;

////////////////////////////////////////////////////////////////////////////////

RegionsToMesh::RegionsToMesh(const std::string& name) : MeshTransformer(name)
{
  options().add("regions", m_regions)
    .pretty_name("Regions")
    .description("Regions to split off into a separate mesh")
    .link_to(&m_regions)
    .mark_basic();
}

void RegionsToMesh::execute()
{
  // This component is meant to split off regions into a separate mesh, to allow partitioning on a per-region basis without much modification to the partitioners.
  throw common::NotImplemented(FromHere(), "RegionsToMesh component is not implemented yet");

  Mesh& mesh = this->mesh();
  // Used nodes, for each dictionary and each region. Regions vector ordered the same way as the regions option value
  std::map<Handle<mesh::Dictionary const>, std::vector<boost::shared_ptr<common::List< Uint>>>> used_nodes;

  for(const auto& region : m_regions)
  {
    const mesh::Mesh& parent_mesh = common::find_parent_component<mesh::Mesh>(*region);
    for(const auto& dict : parent_mesh.dictionaries())
    {
      used_nodes[dict].push_back(build_used_nodes_list(*region, *dict, true, true));
    }
  }


}


//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
