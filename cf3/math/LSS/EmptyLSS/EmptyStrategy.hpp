// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_EmptyStrategy_hpp
#define cf3_Math_LSS_EmptyStrategy_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include "math/LSS/SolutionStrategy.hpp"
#include "math/LSS/LibLSS.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
 *  @file EmptyStrategy.hpp Dummy solver strategy
 *  @author Bart Janssens
 **/
////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

class LSS_API EmptyStrategy : public SolutionStrategy
{
public:

  /// Default constructor
  EmptyStrategy(const std::string& name);

  ~EmptyStrategy();

  /// name of the type
  static std::string type_name () { return "EmptyStrategy"; }

  void set_matrix(const Handle<LSS::Matrix>& matrix);
  void set_rhs(const Handle<LSS::Vector>& rhs);
  void set_solution(const Handle<LSS::Vector>& solution);
  void solve();
  Real compute_residual();
}; // end of class EmptyStrategy

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_EmptyStrategy_hpp
