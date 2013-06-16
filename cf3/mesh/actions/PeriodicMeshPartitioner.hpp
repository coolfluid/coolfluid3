// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_PeriodicMeshPartitioner_hpp
#define cf3_mesh_actions_PeriodicMeshPartitioner_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshTransformer.hpp"

#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
  namespace common { class ActionDirector; }
namespace mesh {
namespace actions {

//////////////////////////////////////////////////////////////////////////////

class PeriodicMeshPartitioner : public MeshTransformer
{
public:
  PeriodicMeshPartitioner(const std::string& name);
  virtual ~PeriodicMeshPartitioner();
  static std::string type_name() { return "PeriodicMeshPartitioner"; }
  virtual void execute();
  
  Handle<MeshTransformer> create_link_periodic_nodes();
private:
  void signal_create_link_periodic_nodes(common::SignalArgs& args);
  Handle<MeshTransformer> m_make_boundary_global;
  Handle<common::ActionDirector> m_periodic_boundary_linkers;
  Handle<MeshTransformer> m_remove_ghosts;
  Handle<MeshTransformer> m_mesh_partitioner;
};


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_PeriodicMeshPartitioner_hpp
