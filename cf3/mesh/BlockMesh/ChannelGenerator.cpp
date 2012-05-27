// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign.hpp>

#include "common/Builder.hpp"
#include "common/Core.hpp"
#include "common/Exception.hpp"
#include "common/EventHandler.hpp"
#include "common/Log.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/Table.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/XML/SignalOptions.hpp"
#include "common/Timer.hpp"

#include "common/PE/Comm.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/BlockMesh/ChannelGenerator.hpp"

namespace cf3 {
namespace mesh {
namespace BlockMesh {

using namespace cf3::common;
using namespace cf3::mesh;

using namespace boost::assign;

ComponentBuilder < ChannelGenerator, Component, LibBlockMesh > ChannelGenerator_Builder;

ChannelGenerator::ChannelGenerator(const std::string& name): MeshGenerator(name)
{
  options().add("nb_parts", PE::Comm::instance().size())
    .description("Total number of partitions (e.g. number of processors)")
    .pretty_name("Number of Partitions");

  options().add("cell_overlap", PE::Comm::instance().size())
    .description("Cell overlap between two adjacent processors")
    .pretty_name("Cell Overlap");

  options().add("x_segments", 10u)
    .description("Number of segments in the X direction")
    .pretty_name("X segments");

  options().add("y_segments_half", 10u)
    .description("Number of segments in the Y direction for one half of the channel")
    .pretty_name("Y segments half");

  options().add("z_segments", 10u)
    .description("Number of segments in the Z direction")
    .pretty_name("Z segments");

  options().add("length", 10.)
    .description("Length in the X direction")
    .pretty_name("Length");

  options().add("half_height", 0.5)
    .description("Channel half height, in the Y-direction")
    .pretty_name("Half Height");

  options().add("width", 10.)
    .description("Channel witdh in the Z-direction")
    .pretty_name("Width");

  options().add("grading", 0.2)
    .description("Grading ratio. Values smaller than one refine towards the wall")
    .pretty_name("Grading Ratio");
}

void ChannelGenerator::execute()
{
  if(is_not_null(get_child("BlockArrays")))
    remove_component("BlockArrays");

  if(is_not_null(get_child("ParallelBlocks")))
    remove_component("ParallelBlocks");

  const Uint x_segs = options().value<Uint>("x_segments");
  const Uint y_segs_half = options().value<Uint>("y_segments_half");
  const Uint z_segs = options().value<Uint>("z_segments");

  const Real length = options().value<Real>("length");
  const Real half_height = options().value<Real>("half_height");
  const Real width = options().value<Real>("width");
  const Real ratio = options().value<Real>("grading");

  BlockArrays& blocks = *create_component<BlockArrays>("BlockArrays");

  Table<Real>& points = *blocks.create_points(3, 12);
  points  << 0.     << -half_height << 0.
          << length << -half_height << 0.
          << 0.     <<  0.          << 0.
          << length <<  0.          << 0.
          << 0.     <<  half_height << 0.
          << length <<  half_height << 0.
          << 0.     << -half_height << width
          << length << -half_height << width
          << 0.     <<  0.          << width
          << length <<  0.          << width
          << 0.     <<  half_height << width
          << length <<  half_height << width;

  Table<Uint>& block_nodes = *blocks.create_blocks(2);
  block_nodes << 0 << 1 << 3 << 2 << 6 << 7 << 9 << 8
              << 2 << 3 << 5 << 4 << 8 << 9 << 11 << 10;

  Table<Uint>& block_subdivisions = *blocks.create_block_subdivisions();
  block_subdivisions << x_segs << y_segs_half << z_segs
                     << x_segs << y_segs_half << z_segs;

  Table<Real>& block_gradings = *blocks.create_block_gradings();
  block_gradings << 1. << 1. << 1. << 1. << 1./ratio << 1./ratio << 1./ratio << 1./ratio << 1. << 1. << 1. << 1.
                 << 1. << 1. << 1. << 1. << ratio    << ratio    << ratio    << ratio    << 1. << 1. << 1. << 1.;

  *blocks.create_patch("bottom", 1) << 0 << 1 << 7 << 6;
  *blocks.create_patch("top", 1) << 4 << 10 << 11 << 5;
  *blocks.create_patch("front", 2) << 0 << 2 << 3 << 1 << 2 << 4 << 5 << 3;
  *blocks.create_patch("back", 2) << 6 << 7 << 9 << 8 << 8 << 9 << 11 << 10;
  *blocks.create_patch("left", 2) << 0 << 6 << 8 << 2 << 2 << 8 << 10 << 4;
  *blocks.create_patch("right", 2) << 1 << 3 << 9 << 7 << 3 << 5 << 11 << 9;

  const Uint nb_parts = options().value<Uint>("nb_parts");

  Mesh& mesh = *m_mesh;

  if(PE::Comm::instance().is_active() && nb_parts > 1)
  {
    const Uint cell_overlap = options().value<Uint>("cell_overlap");
    blocks.options().set("overlap", cell_overlap);
  }

  blocks.partition_blocks(nb_parts, XX);
  blocks.create_mesh(mesh); //--> raises mesh_loaded event inside
}

} // BlockMesh
} // mesh
} // cf3
