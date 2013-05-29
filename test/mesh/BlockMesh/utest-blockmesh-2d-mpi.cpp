// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::BlockMesh::BlockMeshMPI"

#include <boost/assign.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Core.hpp"
#include "common/Log.hpp"
#include "common/List.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"
#include "common/OptionList.hpp"

#include "mesh/BlockMesh/BlockData.hpp"
#include "mesh/Domain.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Field.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Dictionary.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;

using namespace boost::assign;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( BlockMesh2D )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Grid2D )
{
  PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);

  const Uint nb_procs = PE::Comm::instance().size();
  const Uint rank = PE::Comm::instance().rank();
  cf3_assert(nb_procs==8);

  //PE::wait_for_debugger(0);

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

  // Partition the blocks
  blocks.partition_blocks(4, 1);
  blocks.partition_blocks(2, 0);

  // Build the mesh
  Mesh& mesh = *domain.create_component<Mesh>("mesh");
  blocks.create_mesh(mesh);

  // Store element ranks
  Dictionary& elems_P0 = mesh.create_discontinuous_space("elems_P0","cf3.mesh.LagrangeP0");
  Field& elem_rank = elems_P0.create_field("elem_rank");

  boost_foreach(const Handle<Entities>& elements_handle, elems_P0.entities_range())
  {
    Entities& elements = *elements_handle;
    const Space& space = elems_P0.space(elements);
    for (Uint elem=0; elem<elements.size(); ++elem)
    {
      Uint field_idx = space.connectivity()[elem][0];
      elem_rank[field_idx][0] = elements.rank()[elem];
    }
  }

  // Write to disk
  std::vector<URI> fields;
  fields.push_back(elem_rank.uri());
  mesh.write_mesh("utest-blockmesh-2d-mpi_output.pvtu", fields);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

