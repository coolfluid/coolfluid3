// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::BlockMesh::BlockMeshMPI"

#include <boost/assign.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/FindComponents.hpp"
#include "common/Log.hpp"
#include "common/List.hpp"
#include "common/Table.hpp"
#include "common/OptionList.hpp"

#include "mesh/BlockMesh/BlockData.hpp"
#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;

using namespace boost::assign;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( BlockMesh2D )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Grid2D )
{
  Core::instance().environment().options().set("log_level", 4u);

  Domain& domain = *Core::instance().root().create_component<Domain>("domain");

  const Real length = 1.;
  const Real half_height = 1.;
  const Real ratio = 0.2;
  const Uint x_segs = 12;
  const Uint y_segs = 10;

  BlockMesh::BlockArrays& blocks = *domain.create_component<BlockMesh::BlockArrays>("blocks");

  (*blocks.create_points(2, 6)) << 0.     << -half_height
                                << length << -half_height
                                << 0.     <<  0.
                                << length <<  0.
                                << 0.     <<  half_height
                                << length <<  half_height;

  (*blocks.create_blocks(2)) << 0 << 1 << 3 << 2
                             << 2 << 3 << 5 << 4;

  (*blocks.create_block_subdivisions()) << x_segs << y_segs
                                        << x_segs << y_segs;

  (*blocks.create_block_gradings()) << 1. << 1. << 1./ratio << 1./ratio
                                    << 1. << 1. << ratio << ratio;

  *blocks.create_patch("left", 2) << 2 << 0 << 4 << 2;
  *blocks.create_patch("right", 2) << 1 << 3 << 3 << 5;
  *blocks.create_patch("top", 1) << 5 << 4;
  *blocks.create_patch("bottom", 1) << 0 << 1;

  Mesh& mesh = *domain.create_component<Mesh>("mesh");

  blocks.create_mesh(mesh);

  BOOST_CHECK_EQUAL(mesh.dimension(), 2);

  mesh.write_mesh("grid-2d.msh");

  // Test block partitioning
  blocks.partition_blocks(5, 0);
  blocks.partition_blocks(3, 1);
  Handle<Mesh> block_mesh = blocks.create_block_mesh();
  std::vector<URI> fields;
  BOOST_FOREACH(const Field& field, find_components_recursively<Field>(*block_mesh))
  {
    if(field.name() != "coordinates")
      fields.push_back(field.uri());
  }
  block_mesh->write_mesh("grid-2d-parblocks.msh", fields);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

