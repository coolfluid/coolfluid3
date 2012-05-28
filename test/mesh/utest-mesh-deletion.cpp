// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/test/unit_test.hpp>

#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/EventHandler.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"

#include "common/XML/SignalFrame.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"


using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;

BOOST_AUTO_TEST_SUITE( MeshDeletion )

// Solve the Stokes equations with artificial dissipation
BOOST_AUTO_TEST_CASE( DeleteMesh )
{
  // debug output
  Core::instance().environment().options().set("log_level", 3u);

  const Real length = 5.;
  const Real height = 2.;
  const Uint x_segments = 25;
  const Uint y_segments = 10;

  Component& root = Core::instance().root();

  // Setup a domain
  Domain& domain = *root.create_component<Domain>("Domain");

  Handle<Mesh> mesh = domain.create_component<Mesh>("mesh_empty");
  BOOST_CHECK(is_null(mesh) == false);

  domain.remove_component("mesh_empty");
  BOOST_CHECK(is_null(mesh) == true);


  mesh = domain.create_component<Mesh>("generated_rectangle");
  Tools::MeshGeneration::create_rectangle(*mesh, length, height, x_segments, y_segments);
  BOOST_CHECK(is_null(mesh) == false);

  domain.remove_component("generated_rectangle");
  BOOST_CHECK(is_null(mesh) == true);

  if (is_null(mesh))
  {
    // Setup a new mesh
    Tools::MeshGeneration::create_rectangle(*domain.create_component<Mesh>("Mesh2"), length, height, x_segments, y_segments);
    domain.remove_component("Mesh2");
  }
  
  XML::SignalFrame frame;

  Core::instance().terminate();

  // Check if any component pings back. No component should respond
  Core::instance().event_handler().raise_event("ping", frame);
}

BOOST_AUTO_TEST_SUITE_END()
