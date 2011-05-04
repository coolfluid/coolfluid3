// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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

/// CIterate models a Unsteady PDE problem
/// @author Tiago Quintino
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
  
  Uint iter() { return m_iter; }

protected: // data
  
  /// maximum number of iterations this simulation will do
  Uint m_iter;

  Uint m_max_iter;

  bool m_verbose;
};

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CIterate_hpp
