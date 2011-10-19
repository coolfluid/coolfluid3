// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_BlockMesh_ChannelGenerator_hpp
#define cf3_Mesh_BlockMesh_ChannelGenerator_hpp

#include "Common/CF.hpp"
#include "Common/Component.hpp"

#include "Mesh/CMeshGenerator.hpp"

#include "Mesh/BlockMesh/LibBlockMesh.hpp"
#include "Mesh/BlockMesh/BlockData.hpp"

namespace cf3 {
namespace Mesh {

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
} // Mesh
} // cf3

#endif /* CF3_Mesh_BlockMesh_ChannelGenerator_hpp */
