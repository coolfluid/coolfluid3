// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/BasicExceptions.hpp"
#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Region.hpp"

#include "physics/PhysModel.hpp"

#include "solver/SimpleSolver.hpp"
#include "solver/Tags.hpp"

namespace cf3 {
namespace solver {

using namespace common;
using namespace mesh;

common::ComponentBuilder < SimpleSolver, Solver, LibSolver > Builder_SimpleSolver;

////////////////////////////////////////////////////////////////////////////////

SimpleSolver::SimpleSolver(const std::string& name) : Solver(name)
{
}

SimpleSolver::~SimpleSolver()
{
}

////////////////////////////////////////////////////////////////////////////////

void SimpleSolver::mesh_loaded(Mesh& mesh)
{
  m_mesh = mesh.handle<Mesh>();

  // Update the dimensions on the physics
  cf3_assert(mesh.dimension()>0);
  physics().options().set(common::Tags::dimension(), mesh.dimension());
}

////////////////////////////////////////////////////////////////////////////////

Mesh& SimpleSolver::mesh()
{
  if(is_null(m_mesh))
    throw SetupError(FromHere(), "No mesh configured for " + uri().string());

  return *m_mesh;
}

////////////////////////////////////////////////////////////////////////////////


} // solver
} // cf3
