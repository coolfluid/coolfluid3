// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_Actions_CPrintIterationSummary_hpp
#define cf3_Solver_Actions_CPrintIterationSummary_hpp

#include "common/CAction.hpp"

#include "Solver/Actions/LibActions.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh   { class Field; }
namespace Solver {
namespace Actions {

class Solver_Actions_API CPrintIterationSummary : public common::CAction {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CPrintIterationSummary> Ptr;
  typedef boost::shared_ptr<CPrintIterationSummary const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CPrintIterationSummary ( const std::string& name );

  /// Virtual destructor
  virtual ~CPrintIterationSummary() {}

  /// Get the class name
  static std::string type_name () { return "CPrintIterationSummary"; }

  /// execute the action
  virtual void execute ();

private: // data

  boost::weak_ptr<Component> my_norm;
  boost::weak_ptr<Component> my_iter;

};

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3

#endif // cf3_Solver_Actions_CPrintIterationSummary_hpp