// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_SetupSingleSolution_hpp
#define cf3_RDM_SetupSingleSolution_hpp

#include "solver/Action.hpp"

#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace mesh { class Field; }

namespace RDM {


class RDM_API SetupSingleSolution : public cf3::solver::Action {

public: // typedefs

  /// pointers
  
  

public: // functions

  /// Contructor
  /// @param name of the component
  SetupSingleSolution ( const std::string& name );

  /// Virtual destructor
  virtual ~SetupSingleSolution() {}

  /// Get the class name
  static std::string type_name () { return "SetupSingleSolution"; }

  /// execute the action
  virtual void execute ();

};

////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3

#endif // cf3_RDM_SetupSingleSolution_hpp
