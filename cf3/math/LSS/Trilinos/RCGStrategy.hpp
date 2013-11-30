// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_RCGStrategy_hpp
#define cf3_Math_LSS_RCGStrategy_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>

#include "CoordinatesStrategy.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
 *  @file RCGStrategy.hpp Solution strategy to drive recycling Conjugate Gradient
 *  @author Bart Janssens
 **/
////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

class LSS_API RCGStrategy : public CoordinatesStrategy
{
public:
  RCGStrategy(const std::string& name);
  ~RCGStrategy();

  /// name of the type
  static std::string type_name () { return "RCGStrategy"; }

  void set_matrix(const Handle<LSS::Matrix>& matrix);
  void set_rhs(const Handle<LSS::Vector>& rhs);
  void set_solution(const Handle<LSS::Vector>& solution);
  void solve();
  Real compute_residual();
  virtual void set_coordinates(common::PE::CommPattern& cp, const common::Table< Real >& coords, const common::List< Uint >& used_nodes, const std::vector< bool >& periodic_links_active);

private:
  void on_parameters_changed_event(common::SignalArgs& args);
  /// Hide the implementation to avoid pulling in lots of Trilinos headers
  struct Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_RCGStrategy_hpp
