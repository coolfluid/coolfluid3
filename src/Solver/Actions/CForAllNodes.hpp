// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CForAllNodes_hpp
#define CF_Solver_Actions_CForAllNodes_hpp


#include "Solver/Actions/CLoop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
    class CElements;
}
namespace Solver {
namespace Actions {

/////////////////////////////////////////////////////////////////////////////////////

class Solver_Actions_API CForAllNodes : public CLoop
{
public: // typedefs

  typedef boost::shared_ptr< CForAllNodes > Ptr;
  typedef boost::shared_ptr< CForAllNodes const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CForAllNodes ( const std::string& name );

  /// Virtual destructor
  virtual ~CForAllNodes() {}

  /// Get the class name
  static std::string type_name () { return "CForAllNodes"; }

  // functions specific to the CForAllNodes component

  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_Actions_CForAllNodes_hpp
