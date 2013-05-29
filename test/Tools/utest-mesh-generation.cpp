// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the mesh generators"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Core.hpp"

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
  // Setup document structure and mesh
  Component& root = Core::instance().root();

  Handle<Mesh> mesh = root.create_component<Mesh>("mesh");
  Tools::MeshGeneration::create_rectangle(*mesh, 10., 5., 5, 5);

  boost::shared_ptr<MeshWriter> writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  root.add_component(writer);
  writer->write_from_to(*mesh, "grid_2d.msh");
}

BOOST_AUTO_TEST_SUITE_END()
