// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CSimpleSolver_hpp
#define CF_Solver_CSimpleSolver_hpp

#include <boost/weak_ptr.hpp>

#include "Solver/CSolver.hpp"
#include "Solver/LibSolver.hpp"

namespace CF {

namespace Mesh { class CMesh; }

namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// Base class for solvers that work under the following assumptions:
/// - There is only one mesh for the solution. Whenever a mesh is loaded into the domain, that mesh is used by the solver
/// - Fields are created when the mesh is loaded, based on the physical model
/// An option named "physical_model" is created, if this is not set automatic field creation does nothing
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
  virtual void mesh_loaded(Mesh::CMesh& mesh);

protected:
  /// Checked access to the mesh
  Mesh::CMesh& mesh();

private:
  boost::weak_ptr<Mesh::CMesh> m_mesh;
};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

#endif // CF_Solver_CSimpleSolver_hpp
