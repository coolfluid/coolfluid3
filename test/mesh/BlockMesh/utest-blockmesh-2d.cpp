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
#include "common/Log.hpp"
#include "common/List.hpp"

#include "mesh/BlockMesh/BlockData.hpp"
#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshWriter.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;

using namespace boost::assign;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( BlockMesh2D )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Grid2D )
{
  boost::shared_ptr< MeshWriter > writer =  build_component_abstract_type<MeshWriter>("cf3.mesh.VTKLegacy.Writer", "writer");
  Domain& domain = *Core::instance().root().create_component<Domain>("domain");
  domain.add_component(writer);

  const Real length = 1.;
  const Real half_height = 1.;
  const Real ratio = 0.2;
  const Uint x_segs = 12;
  const Uint y_segs = 10;

  BlockMesh::BlockData& blocks = *domain.create_component<BlockMesh::BlockData>("blocks");

  blocks.dimension = 2;
  blocks.scaling_factor = 1.;

  blocks.points += list_of(0.    )(-half_height)
                 , list_of(length)(-half_height)
                 , list_of(0.    )( 0.         )
                 , list_of(length)( 0.         )
                 , list_of(0.    )( half_height)
                 , list_of(length)( half_height);

  blocks.block_points += list_of(0)(1)(3)(2),
                         list_of(2)(3)(5)(4);
  blocks.block_subdivisions += list_of(x_segs)(y_segs),
                               list_of(x_segs)(y_segs);
  blocks.block_gradings += list_of(1.)(1.)(1./ratio)(1./ratio),
                           list_of(1.)(1.)(ratio)(ratio);

  blocks.patch_names += "left", "right", "top",  "bottom";
  blocks.patch_types += "wall", "wall",  "wall", "wall";
  blocks.patch_points += list_of(2)(0)(4)(2),
                         list_of(1)(3)(3)(5),
                         list_of(5)(4),
                         list_of(0)(1);

  blocks.block_distribution += 0, 2;

  domain.add_component(writer);
  Mesh& mesh = *domain.create_component<Mesh>("mesh");

  BlockMesh::build_mesh(blocks, mesh);

  BOOST_CHECK_EQUAL(mesh.dimension(), 2);

  writer->write_from_to(mesh, URI("grid-2d.vtk"));

  // Test block partitioning
  BlockMesh::BlockData& parallel_blocks = *domain.create_component<BlockMesh::BlockData>("parallel_blocks");
  BlockMesh::partition_blocks(blocks, 4, XX, parallel_blocks);

  Mesh& parallel_block_mesh = *domain.create_component<Mesh>("parallel_block_mesh");
  BlockMesh::create_block_mesh(parallel_blocks, parallel_block_mesh);
  writer->write_from_to(parallel_block_mesh, URI("grid-2d-parblocks.vtk"));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

