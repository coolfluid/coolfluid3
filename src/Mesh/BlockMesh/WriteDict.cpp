// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/range.hpp>

#include "Common/StreamHelpers.hpp"

#include "WriteDict.hpp"

using namespace cf3::common;

namespace cf3 {
namespace Mesh {
namespace BlockMesh {

std::ostream& operator<<(std::ostream& os, const BlockData::IndicesT& data)
{
  print_vector(os, data, " ", "(", ")");
  return os;
}

std::ostream& operator<<(std::ostream& os, const BlockData::PointT& data)
{
  print_vector(os, data, " ", "(", ")");
  return os;
}

std::ostream& operator<<(std::ostream& os, const boost::iterator_range<BlockData::IndicesT::const_iterator>& data)
{
  print_vector(os, data, " ", "(", ")");
  return os;
}

/// Write an BlockMesh blockMeshDict to the supplied output stream
std::ostream& operator<<(std::ostream& os, const BlockData& block_data)
{
  // header
  os << "FoamFile\n{\n  version     2.0;\n  format      ascii;\n  class       dictionary;\n  object      blockMeshDict;\n}\n\n";
  
  // scaling
  os << "convertToMeters " << block_data.scaling_factor << ";\n\n";
  
  // points
  os << "vertices\n(\n";
  BOOST_FOREACH(const BlockData::PointT& point, block_data.points)
  {
    os << "  " << point << "\n";
  }
  os << ");\n\n";
  
  // blocks
  os << "blocks\n(\n";
  const Uint nb_blocks = block_data.block_points.size();
  for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
  {
    os << "  hex " << block_data.block_points[block_idx] << " "
       << block_data.block_subdivisions[block_idx]
       << " edgeGrading " << block_data.block_gradings[block_idx]
       << "\n";
  }
  os << ");\n\n";
  
  // patches
  os << "patches\n(\n";
  const Uint nb_patches = block_data.patch_names.size();
  for(Uint patch_idx = 0; patch_idx != nb_patches; ++patch_idx)
  {
    os << "  " << block_data.patch_types[patch_idx] << " " << block_data.patch_names[patch_idx] << "\n  (\n";
    const BlockData::IndicesT& patch_points = block_data.patch_points[patch_idx];
    const Uint sub_patch_size = patch_points.size();
    for(Uint sub_patch_idx = 0; sub_patch_idx != sub_patch_size; sub_patch_idx += 4)
    {
      os << "    " <<  boost::make_iterator_range(patch_points.begin() + sub_patch_idx, patch_points.begin() + sub_patch_idx + 4) << "\n";
    }
    os << "  )\n";
  }
  os << ");\n";
  
  return os;
}
  
} // BlockMesh
} // Mesh
} // cf3
