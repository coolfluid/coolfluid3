// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_CreateSDFields_hpp
#define cf3_sdm_CreateSDFields_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/Action.hpp"
#include "sdm/LibSDM.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

//////////////////////////////////////////////////////////////////////////////

/// This class defines an action that creates a space in the mesh
/// with sdm shape functions, and a configurable order of polynomial.
/// Default polynomial order = 0.
/// that returns information about the mesh
/// @author Willem Deconinck
class sdm_API CreateSDFields : public solver::Action
{
public: // functions

  /// constructor
  CreateSDFields( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "CreateSDFields"; }

  virtual void execute();

}; // end CreateSDFields


////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_CreateSDFields_hpp
