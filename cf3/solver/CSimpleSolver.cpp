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

#include "solver/CSimpleSolver.hpp"
#include "solver/Tags.hpp"

namespace cf3 {
namespace solver {

using namespace common;
using namespace mesh;

common::ComponentBuilder < CSimpleSolver, CSolver, LibSolver > Builder_CSimpleSolver;

////////////////////////////////////////////////////////////////////////////////

CSimpleSolver::CSimpleSolver(const std::string& name) : CSolver(name)
{
}

CSimpleSolver::~CSimpleSolver()
{
}

////////////////////////////////////////////////////////////////////////////////

void CSimpleSolver::mesh_loaded(Mesh& mesh)
{
  m_mesh = mesh.handle<Mesh>();

  // Update the dimensions on the physics
  physics().options().configure_option(common::Tags::dimension(), mesh.topology().geometry_fields().coordinates().row_size());
}

////////////////////////////////////////////////////////////////////////////////

Mesh& CSimpleSolver::mesh()
{
  if(is_null(m_mesh))
    throw SetupError(FromHere(), "No mesh configured for " + uri().string());

  return *m_mesh;
}

////////////////////////////////////////////////////////////////////////////////


} // solver
} // cf3
