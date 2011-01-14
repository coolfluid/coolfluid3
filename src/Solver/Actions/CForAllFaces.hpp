// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CForAllFaces_hpp
#define CF_Solver_Actions_CForAllFaces_hpp


#include "Solver/Actions/CLoop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
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
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Actions_CForAllFaces_hpp
