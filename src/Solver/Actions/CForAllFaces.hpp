// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_Actions_CForAllFaces_hpp
#define cf3_Solver_Actions_CForAllFaces_hpp


#include "Solver/Actions/CLoop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Solver {
namespace Actions {

/////////////////////////////////////////////////////////////////////////////////////

class Solver_Actions_API CForAllFaces : public CLoop
{
public: // typedefs

  typedef boost::shared_ptr< CForAllFaces > Ptr;
  typedef boost::shared_ptr< CForAllFaces const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CForAllFaces ( const std::string& name );

  /// Virtual destructor
  virtual ~CForAllFaces() {}

  /// Get the class name
  static std::string type_name () { return "CForAllFaces"; }

  // functions specific to the CForAllFaces component

  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF3_Actions_CForAllFaces_hpp
