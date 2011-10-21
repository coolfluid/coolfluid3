// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_CSimpleSolver_hpp
#define cf3_Solver_CSimpleSolver_hpp

#include <boost/weak_ptr.hpp>

#include "Solver/CSolver.hpp"
#include "Solver/LibSolver.hpp"

namespace cf3 {

namespace mesh { class CMesh; }

namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// Base class for solvers that work under the following assumptions:
/// - There is only one mesh for the solution. Whenever a mesh is loaded into the domain, that mesh is used by the solver
/// @author Bart Janssens
class Solver_API CSimpleSolver : public CSolver {

public: // typedefs

  /// type of pointer to Component
  typedef boost::shared_ptr<CSimpleSolver> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<CSimpleSolver const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CSimpleSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~CSimpleSolver();

  /// Get the class name
  static std::string type_name () { return "CSimpleSolver"; }

  /// When a mesh is loaded into the domain, set this as the mesh returned by mesh() and
  /// create the fields, based on registered fields in the physical model (if any)
  virtual void mesh_loaded(mesh::CMesh& mesh);

protected:
  /// Checked access to the mesh
  mesh::CMesh& mesh();

  /// Weak pointer to the mesh that was last loaded, or expired otherwise
  boost::weak_ptr<mesh::CMesh> m_mesh;
};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // cf3

#endif // cf3_Solver_CSimpleSolver_hpp
