// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_WallDistance_hpp
#define cf3_mesh_actions_WallDistance_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshTransformer.hpp"
#include "mesh/Region.hpp"

#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

//////////////////////////////////////////////////////////////////////////////

class WallDistance : public MeshTransformer
{
public:
  WallDistance(const std::string& name);
  static std::string type_name() { return "WallDistance"; }
  virtual void execute();

private:
  /// Wall regions to operate over
  std::vector< Handle<Region> > m_regions;
};


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_WallDistance_hpp
