// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_TrilinosStratimikosStrategy_hpp
#define cf3_Math_LSS_TrilinosStratimikosStrategy_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>

#include "math/LSS/SolutionStrategy.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file TrilinosStratimikosStrategy.hpp Drives Trilinos linear solvers through Stratimikos
  @author Bart Janssens
**/
////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

class LSS_API TrilinosStratimikosStrategy : public SolutionStrategy
{
public:

  /// Default constructor
  TrilinosStratimikosStrategy(const std::string& name);

  ~TrilinosStratimikosStrategy();

  /// name of the type
  static std::string type_name () { return "TrilinosStratimikosStrategy"; }

  void set_matrix(const Handle<LSS::Matrix>& matrix);
  void set_rhs(const Handle<LSS::Vector>& rhs);
  void set_solution(const Handle<LSS::Vector>& solution);
  void solve();
  Real compute_residual();

  /// Construct default parameters using the builder for a ParameterListDefaults object.
  void set_default_parameters(const std::string& builder_name);

private:
  void on_parameters_changed_event(common::SignalArgs& args);

  /// Hide the implementation to avoid pulling in lots of Trilinos headers
  struct Implementation;
  boost::scoped_ptr<Implementation> m_implementation;

}; // end of class TrilinosStratimikosStrategy

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_TrilinosStratimikosStrategy_hpp
