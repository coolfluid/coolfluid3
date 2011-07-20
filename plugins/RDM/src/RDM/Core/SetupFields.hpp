// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_SetupFields_hpp
#define CF_RDM_SetupFields_hpp

#include "Solver/Action.hpp"

#include "RDM/Core/LibCore.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Mesh { class CField; }

namespace RDM {
namespace Core {

class RDM_Core_API SetupFields : public CF::Solver::Action {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<SetupFields> Ptr;
  typedef boost::shared_ptr<SetupFields const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  SetupFields ( const std::string& name );

  /// Virtual destructor
  virtual ~SetupFields() {}

  /// Get the class name
  static std::string type_name () { return "SetupFields"; }

  /// execute the action
  virtual void execute ();

};

////////////////////////////////////////////////////////////////////////////////

} // Core
} // RDM
} // CF

#endif // CF_RDM_SetupFields_hpp
