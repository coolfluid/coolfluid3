// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_BC_hpp
#define CF_FVM_BC_hpp

#include "Solver/Actions/CLoopOperation.hpp"
#include "FVM/LibFVM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace FVM {

///////////////////////////////////////////////////////////////////////////////////////

class FVM_API BC : public Solver::Actions::CLoopOperation
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

} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_BC_hpp
