// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"
#include "Common/Log.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CRegion.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/VariableManager.hpp"

#include "Solver/CreateFields.hpp"
#include "Solver/CSimpleSolver.hpp"
#include "Solver/Tags.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Mesh;

////////////////////////////////////////////////////////////////////////////////

CSimpleSolver::CSimpleSolver(const std::string& name) : CSolver(name)
{
}

CSimpleSolver::~CSimpleSolver()
{
}

////////////////////////////////////////////////////////////////////////////////

void CSimpleSolver::mesh_loaded(CMesh& mesh)
{
  m_mesh = mesh.as_ptr<CMesh>();

  Physics::PhysModel& phys_model = physics();

  // Update the dimensions on the physics
  phys_model.variable_manager().configure_option("dimensions", mesh.topology().geometry().dim());

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

} // Solver
} // CF
