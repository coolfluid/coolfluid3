// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CreateFields_hpp
#define CF_Solver_CreateFields_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/LibSolver.hpp"

namespace CF {

  namespace Mesh    { class CMesh; }
  namespace Physics { class PhysModel; }

namespace Solver {

/// Create the fields for variables that are registered with the VariableManager in the passed physical model
/// @param mesh The mesh in which the fields will be created
/// @param physical_model The physical model that contains descriptions of the variables
/// Does nothing if a compatible field existed already, throws an exception if an incompatible field was found.
void Solver_API create_fields(Mesh::CMesh& mesh, Physics::PhysModel& physical_model);


////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CreateFields_hpp
