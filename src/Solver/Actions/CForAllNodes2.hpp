// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CForAllNodes2_hpp
#define CF_Solver_Actions_CForAllNodes2_hpp


#include "Solver/Actions/CLoop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Solver {
namespace Actions {

/////////////////////////////////////////////////////////////////////////////////////

class Solver_Actions_API CForAllNodes2 : public CLoop
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

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_Actions_CForAllNodes2_hpp
