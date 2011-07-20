// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"
#include "Common/Log.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CRegion.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/VariableManager.hpp"

#include "Solver/CreateFields.hpp"
#include "Solver/CSimpleSolver.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Mesh;

////////////////////////////////////////////////////////////////////////////////

CSimpleSolver::CSimpleSolver(const std::string& name) : CSolver(name)
{
  m_options.add_option( OptionComponent<Physics::PhysModel>::create("physical_model", &m_physics) )
              ->set_pretty_name("Physical Model")
              ->set_description("Physical Model");
}

CSimpleSolver::~CSimpleSolver()
{
}

////////////////////////////////////////////////////////////////////////////////

void CSimpleSolver::mesh_loaded(CMesh& mesh)
{
  m_mesh = mesh.as_ptr<CMesh>();
  
  if(m_physics.expired())
  {
    CFdebug << "Not creating fields because physical model is not set for " << uri().string() << CFendl;
    return;
  }
  
  Physics::PhysModel& phys_model = *m_physics.lock();
  
  // Update the dimensions on the physics
  phys_model.variable_manager().configure_option("dimensions", mesh.topology().nodes().dim());
  
  // Create the fields
  create_fields(mesh, phys_model);
}

////////////////////////////////////////////////////////////////////////////////

CMesh& CSimpleSolver::mesh()
{
  if(m_mesh.expired())
    throw SetupError(FromHere(), "No mesh configured for " + uri().string());
  
  return *m_mesh.lock();
}

////////////////////////////////////////////////////////////////////////////////

Physics::PhysModel& CSimpleSolver::physics()
{
  if(m_physics.expired())
    throw SetupError(FromHere(), "No physical model configured for " + uri().string());
  
  return *m_physics.lock();
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
