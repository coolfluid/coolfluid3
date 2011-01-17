// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CForAllElements2_hpp
#define CF_Solver_Actions_CForAllElements2_hpp


#include "Solver/Actions/CLoop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Solver {
namespace Actions {

/////////////////////////////////////////////////////////////////////////////////////

class Solver_Actions_API CForAllElements2 : public CLoop
{
public: // typedefs

  typedef boost::shared_ptr< CForAllElements2 > Ptr;
  typedef boost::shared_ptr< CForAllElements2 const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CForAllElements2 ( const std::string& name );

  /// Virtual destructor
  virtual ~CForAllElements2() {}

  /// Get the class name
  static std::string type_name () { return "CForAllElements2"; }

  // functions specific to the CForAllElements2 component

  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Actions_CForAllElements2_hpp
