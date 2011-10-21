// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_BlockMesh_ChannelGenerator_hpp
#define cf3_mesh_BlockMesh_ChannelGenerator_hpp

#include "common/CF.hpp"
#include "common/Component.hpp"

#include "mesh/CMeshGenerator.hpp"

#include "mesh/BlockMesh/LibBlockMesh.hpp"
#include "mesh/BlockMesh/BlockData.hpp"

namespace cf3 {
namespace mesh {

class CMesh;

namespace BlockMesh {


////////////////////////////////////////////////////////////////////////////////

/// Generate parallel 3D channels with grading towards the wall
class BlockMesh_API ChannelGenerator : public CMeshGenerator
{
public:
  typedef boost::shared_ptr<ChannelGenerator> Ptr;
  typedef boost::shared_ptr<ChannelGenerator const> ConstPtr;
  
  ChannelGenerator(const std::string& name);
  
  static std::string type_name () { return "ChannelGenerator"; }
  
  virtual void execute();
};

} // BlockMesh
} // mesh
} // cf3

#endif /* CF3_Mesh_BlockMesh_ChannelGenerator_hpp */
