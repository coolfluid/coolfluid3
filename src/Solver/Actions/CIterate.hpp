// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CIterate_hpp
#define CF_Solver_CIterate_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/Actions/LibActions.hpp"
#include "Common/CAction.hpp"

namespace CF {
namespace Solver {
namespace Actions {

////////////////////////////////////////////////////////////////////////////////

/// @brief Action component that iteratively executes all contained actions.
///
/// To stop iterating, the configuration "max_iter" can be specified for the amount
/// of iterations, or a stop-criterion, derived from the type Solver::Actions::CCriterion
///
/// @author Willem Deconinck
class Solver_Actions_API CIterate : public Common::CAction {

public: // typedefs

  typedef boost::shared_ptr<CIterate> Ptr;
  typedef boost::shared_ptr<CIterate const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CIterate ( const std::string& name );

  /// Virtual destructor
  virtual ~CIterate();

  /// Get the class name
  static std::string type_name () { return "CIterate"; }

  /// Simulates this model
  virtual void execute();

  /// access to iteration number
  Uint iter() const { return m_iter; }

protected: // data

  /// count of the iteration
  Uint m_iter;

  /// maximum number of iterations
  Uint m_max_iter;

  /// flag to output iteration info
  bool m_verbose;
};

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CIterate_hpp
