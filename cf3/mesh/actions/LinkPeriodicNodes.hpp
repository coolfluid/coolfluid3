// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_LinkPeriodicNodes_hpp
#define cf3_mesh_actions_LinkPeriodicNodes_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshTransformer.hpp"
#include "mesh/Region.hpp"

#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

//////////////////////////////////////////////////////////////////////////////

class LinkPeriodicNodes : public MeshTransformer
{
public:
  LinkPeriodicNodes(const std::string& name);
  static std::string type_name() { return "LinkPeriodicNodes"; }
  virtual void execute();

private:

  // Region of which the nodes will be replaced by the corresponding nodes from destination_region
  Handle<Region> m_source_region;
  Handle<Region> m_destination_region;
  std::vector<Real> m_translation_vector;

};


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_LinkPeriodicNodes_hpp
