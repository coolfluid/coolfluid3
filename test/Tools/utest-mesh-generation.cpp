// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the mesh generators"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Core.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/CRoot.hpp"
#include "Common/LibLoader.hpp"
#include "Common/OSystem.hpp"

#include "Mesh/CMeshWriter.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;

BOOST_AUTO_TEST_SUITE( MeshGenerationSuite )

/// Test creation of a 2D grid
BOOST_AUTO_TEST_CASE( CreateGrid )
{
  // Load the required libraries (we assume the working dir is the binary path)
  LibLoader& loader = *OSystem::instance().lib_loader();
  
  const std::vector< boost::filesystem::path > lib_paths = boost::assign::list_of("../../src/Mesh/Gmsh");
  loader.set_search_paths(lib_paths);
  
  loader.load_library("coolfluid_mesh_gmsh");
  
  // Setup document structure and mesh
  CRoot& root = Core::instance().root();
  
  CMesh::Ptr mesh = root.create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_rectangle(*mesh, 10., 5., 5, 5);
  
  CMeshWriter::Ptr writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  root.add_component(writer);
  boost::filesystem::path output_file("grid_2d.msh");
  writer->write_from_to(mesh, output_file);
}

BOOST_AUTO_TEST_SUITE_END()
