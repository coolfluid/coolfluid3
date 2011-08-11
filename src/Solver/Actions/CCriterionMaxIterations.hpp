// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CCriterionMaxIterations_hpp
#define CF_Solver_Actions_CCriterionMaxIterations_hpp

#include "Solver/Actions/LibActions.hpp"
#include "Solver/Actions/CCriterion.hpp"

namespace CF {
namespace Solver {
namespace Actions {

////////////////////////////////////////////////////////////////////////////////////////////

/// Criterion for maximum number of iterations
/// @author Willem Deconinck
/// @author Tiago Quintino
class Solver_Actions_API CCriterionMaxIterations : public CCriterion {

public: // typedefs

  typedef boost::shared_ptr<CCriterionMaxIterations> Ptr;
  typedef boost::shared_ptr<CCriterionMaxIterations const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CCriterionMaxIterations ( const std::string& name );

  /// Virtual destructor
  virtual ~CCriterionMaxIterations();

  /// Get the class name
  static std::string type_name () { return "CCriterionMaxIterations"; }

  /// Simulates this model
  virtual bool operator()();

private:

  /// component where to access the current iteration
  boost::weak_ptr<Component> m_iter_comp;

};

////////////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

#endif // CF_Solver_Actions_CCriterionMaxIterations_hpp
