// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_SolutionStrategy_hpp
#define cf3_Math_LSS_SolutionStrategy_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

#include "math/LSS/LibLSS.hpp"
#include "math/LSS/BlockAccumulator.hpp"
#include "math/LSS/Matrix.hpp"
#include "math/LSS/Vector.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file SolutionStrategy.hpp Base interface for a strategy to solve a linear system
  @author Bart Janssens
**/
////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

class LSS_API SolutionStrategy : public common::Component
{
public:

  /// name of the type
  static std::string type_name () { return "SolutionStrategy"; }

  /// Default constructor
  SolutionStrategy(const std::string& name) : Component(name)
  {
  }

  /// Set the system matrix for the linear system to solve
  virtual void set_matrix(const Handle<LSS::Matrix>& matrix) = 0;

  /// Set the right hand side of the linear system to solve
  virtual void set_rhs(const Handle<LSS::Vector>& rhs) = 0;

  /// Set the solution vector storage
  virtual void set_solution(const Handle<LSS::Vector>& solution) = 0;

  /// Solve the system
  virtual void solve() = 0;

  virtual Real compute_residual() = 0;

}; // end of class SolutionStrategy

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_SolutionStrategy_hpp
