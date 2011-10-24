// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_CForAllNodes2_hpp
#define cf3_solver_actions_CForAllNodes2_hpp


#include "solver/actions/CLoop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

/////////////////////////////////////////////////////////////////////////////////////

class solver_actions_API CForAllNodes2 : public CLoop
{
public: // typedefs

  typedef boost::shared_ptr< CForAllNodes2 > Ptr;
  typedef boost::shared_ptr< CForAllNodes2 const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CForAllNodes2 ( const std::string& name );

  /// Virtual destructor
  virtual ~CForAllNodes2() {}

  /// Get the class name
  static std::string type_name () { return "CForAllNodes2"; }

  // functions specific to the CForAllNodes2 component

  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_CForAllNodes2_hpp
