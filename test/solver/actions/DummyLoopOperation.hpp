// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_DummyLoopOperation_hpp
#define cf3_solver_actions_DummyLoopOperation_hpp

#include "mesh/Elements.hpp"

#include "solver/actions/LoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace TestActions {

///////////////////////////////////////////////////////////////////////////////////////

class DummyLoopOperation : public solver::actions::LoopOperation {

public: // typedefs

  /// pointers
  
  

public: // functions
  /// Contructor
  /// @param name of the component
  DummyLoopOperation ( const std::string& name );

  /// Virtual destructor
  virtual ~DummyLoopOperation() {}

  /// Get the class name
  static std::string type_name () { return "DummyLoopOperation"; }

  /// execute the action
  virtual void execute ();


private: // data

};

/////////////////////////////////////////////////////////////////////////////////////

} // TestActions
} // cf3

#endif // cf3_TestActions_DummyLoopOperation_hpp
