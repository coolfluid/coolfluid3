// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_YPlus_hpp
#define cf3_mesh_actions_YPlus_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/Action.hpp"
#include "solver/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

//////////////////////////////////////////////////////////////////////////////

class YPlus : public Action
{
public:
  YPlus(const std::string& name);
  static std::string type_name() { return "YPlus"; }
  virtual void execute();
private:
  std::vector<std::vector<RealVector>> m_normals;
  std::vector<std::vector<Real>> m_wall_velocity_gradient;
  std::vector<Real> m_wall_velocity_gradient_nodal;
  std::map<const mesh::Entities*, Uint> m_entities_map;
};


////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_YPlus_hpp
