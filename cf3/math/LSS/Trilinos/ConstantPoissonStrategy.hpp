// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_ConstantPoissonStrategy_hpp
#define cf3_Math_LSS_ConstantPoissonStrategy_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>

#include "math/LSS/SolutionStrategy.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
 *  @file ConstantPoissonStrategy.hpp Optimized solution strategy to solve a Poisson equation with the same matrix multiple times
 *  @author Bart Janssens
 **/
////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

class LSS_API ConstantPoissonStrategy : public SolutionStrategy
{
public:
  ConstantPoissonStrategy(const std::string& name);
  ~ConstantPoissonStrategy();

  /// name of the type
  static std::string type_name () { return "ConstantPoissonStrategy"; }

  void set_matrix(const Handle<LSS::Matrix>& matrix);
  void set_rhs(const Handle<LSS::Vector>& rhs);
  void set_solution(const Handle<LSS::Vector>& solution);
  void solve();
  Real compute_residual();

private:
  void on_parameters_changed_event(common::SignalArgs& args);
  /// Hide the implementation to avoid pulling in lots of Trilinos headers
  struct Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_ConstantPoissonStrategy_hpp
