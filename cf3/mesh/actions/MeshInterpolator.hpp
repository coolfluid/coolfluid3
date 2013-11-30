// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_MeshInterpolator_hpp
#define cf3_mesh_actions_MeshInterpolator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Action.hpp"

#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

//////////////////////////////////////////////////////////////////////////////

/// Interpolate all continuous fields from the source mesh to the target mesh, creating them if needed
/// Be aware that this modifies the source mesh to make all elements global in parallel
class MeshInterpolator : public common::Action
{
public:
  MeshInterpolator(const std::string& name);
  static std::string type_name() { return "MeshInterpolator"; }
  virtual void execute();
};


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_MeshInterpolator_hpp
