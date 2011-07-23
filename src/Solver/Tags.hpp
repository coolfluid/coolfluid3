// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Tags_hpp
#define CF_Solver_Tags_hpp

#include "Solver/LibSolver.hpp"

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////////////////

/// This class defines common tags for use in solvers
/// @author Tiago Quintino
/// @author Bart Janssens
class Solver_API Tags : public NonInstantiable<Tags> {
public:

  /// Name of the option that points to the domain
  static const char * domain();

  /// Name of the option that points to the solver
  static const char * solver();

  /// Name of the option that points to the mesh
  static const char * mesh();

  /// Name of the option that allows setting the regions to operate on
  static const char * regions();

  /// Name of the option that points to the time tracking component
  static const char * time();

  /// Name of the option that points to the PhysModel
  static const char * physical_model();

}; // Tags

////////////////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

#endif // CF_Solver_Tags_hpp
