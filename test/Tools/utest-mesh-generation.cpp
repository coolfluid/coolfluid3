// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the mesh generators"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Core.hpp"

#include "common/Root.hpp"
#include "common/LibLoader.hpp"
#include "common/OSystem.hpp"

#include "mesh/MeshWriter.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;

BOOST_AUTO_TEST_SUITE( MeshGenerationSuite )

/// Test creation of a 2D grid
BOOST_AUTO_TEST_CASE( CreateGrid )
{
  // Load the required libraries (we assume the working dir is the binary path)
  LibLoader& loader = *OSystem::instance().lib_loader();

  const std::vector< boost::filesystem::path > lib_paths = boost::assign::list_of("../../cf3/mesh/gmsh");
  loader.set_search_paths(lib_paths);

  loader.load_library("coolfluid_mesh_gmsh");

  // Setup document structure and mesh
  Root& root = Core::instance().root();

  Mesh& mesh = root.create_component<Mesh>("mesh");
  Tools::MeshGeneration::create_rectangle(mesh, 10., 5., 5, 5);

  MeshWriter::Ptr writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  root.add_component(writer);
  writer->write_from_to(mesh, "grid_2d.msh");
}

BOOST_AUTO_TEST_SUITE_END()
