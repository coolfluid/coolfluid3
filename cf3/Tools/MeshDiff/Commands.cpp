// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/Core.hpp"
#include "common/Root.hpp"
#include "common/Group.hpp"
#include "common/BuildInfo.hpp"
#include "common/CFactory.hpp"
#include "common/Builder.hpp"
 
#include "common/Foreach.hpp"
#include "common/Action.hpp"
#include "common/FindComponents.hpp"

#include "mesh/LoadMesh.hpp"

#include "Tools/MeshDiff/Commands.hpp"
#include "Tools/MeshDiff/MeshDiff.hpp"

namespace cf3 {
namespace Tools {
namespace MeshDiff {

  using namespace boost;
  using namespace boost::program_options;

  using namespace cf3::common;
  using namespace cf3::mesh;

////////////////////////////////////////////////////////////////////////////////

Commands::Commands()
{	
}

////////////////////////////////////////////////////////////////////////////////

Commands::commands_description Commands::description()
{
  commands_description desc("MeshDiff Commands");
  desc.add_options()
  ("compare",    value< std::vector<std::string> >()->multitoken()->notifier(boost::bind(&compare,_1)), "compare meshes")
  ;
  return desc;
}

////////////////////////////////////////////////////////////////////////////////

void Commands::compare(const std::vector<std::string>& params)
{
  Component::Ptr meshes_ptr = Core::instance().root().get_child_ptr("Meshes");
  if (is_null(meshes_ptr))
    meshes_ptr = Core::instance().root().create_component_ptr<Group>("Meshes");
  Group& meshes = meshes_ptr->as_type<Group>();

  Component::Ptr mesh_loader_ptr = Core::instance().root().get_child_ptr("mesh_loader");
  if (is_null(mesh_loader_ptr))
    mesh_loader_ptr = Core::instance().root().create_component_ptr<LoadMesh>("mesh_loader");
  LoadMesh& mesh_loader = mesh_loader_ptr->as_type<LoadMesh>();

  std::vector<Mesh::Ptr> mesh_vector;
  boost_foreach(const std::string& file_str, params)
  {
    URI file(file_str);
    Mesh::Ptr mesh = mesh_loader.load_mesh(file);
    mesh->rename(file.name());
    meshes.add_component(mesh);
    mesh_vector.push_back(mesh);
  }

  Mesh& reference_mesh = *mesh_vector[0];
  for (Uint i=1; i<mesh_vector.size(); ++i)
  {
    CFinfo << "Comparing " << reference_mesh.name() << " to " << mesh_vector[i]->name() << CFendl;
    
    MeshDiff::diff(reference_mesh,*mesh_vector[i],100);
  }
  
}


////////////////////////////////////////////////////////////////////////////////

} // MeshDiff
} // Tools
} // cf3
