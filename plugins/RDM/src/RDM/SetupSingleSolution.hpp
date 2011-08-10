// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_SetupSingleSolution_hpp
#define CF_RDM_SetupSingleSolution_hpp

#include "Solver/Action.hpp"

#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Mesh { class Field; }

namespace RDM {


class RDM_API SetupSingleSolution : public CF::Solver::Action {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<SetupSingleSolution> Ptr;
  typedef boost::shared_ptr<SetupSingleSolution const> ConstPtr;

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
} // CF

#endif // CF_RDM_SetupSingleSolution_hpp
