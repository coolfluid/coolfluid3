// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_Core_BC_hpp
#define CF_FVM_Core_BC_hpp

#include "Solver/Actions/CLoopOperation.hpp"
#include "FVM/Core/LibCore.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace FVM {
namespace Core {

///////////////////////////////////////////////////////////////////////////////////////

class FVM_Core_API BC : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<BC> Ptr;
  typedef boost::shared_ptr<BC const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  BC ( const std::string& name ) : Solver::Actions::CLoopOperation(name) {}

  /// Virtual destructor
  virtual ~BC () {}

  /// Get the class name
  static std::string type_name () { return "BC"; }

  /// execute the action
  virtual void execute () = 0;

};

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_Core_BC_hpp
