// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_SimpleSolver_hpp
#define cf3_solver_SimpleSolver_hpp

#include <boost/weak_ptr.hpp>

#include "solver/Solver.hpp"
#include "solver/LibSolver.hpp"

namespace cf3 {

namespace mesh { class Mesh; }

namespace solver {

////////////////////////////////////////////////////////////////////////////////

/// Base class for solvers that work under the following assumptions:
/// - There is only one mesh for the solution. Whenever a mesh is loaded into the domain, that mesh is used by the solver
/// @author Bart Janssens
class solver_API SimpleSolver : public Solver {

public: // functions

  /// Contructor
  /// @param name of the component
  SimpleSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~SimpleSolver();

  /// Get the class name
  static std::string type_name () { return "SimpleSolver"; }

  /// When a mesh is loaded into the domain, set this as the mesh returned by mesh() and
  /// create the fields, based on registered fields in the physical model (if any)
  virtual void mesh_loaded(mesh::Mesh& mesh);

protected:
  /// Checked access to the mesh
  mesh::Mesh& mesh();

  /// Weak pointer to the mesh that was last loaded, or expired otherwise
  Handle<mesh::Mesh> m_mesh;
};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

#endif // cf3_solver_SimpleSolver_hpp
