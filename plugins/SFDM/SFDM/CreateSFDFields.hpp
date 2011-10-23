// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_CreateSFDFields_hpp
#define cf3_SFDM_CreateSFDFields_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/Action.hpp"
#include "SFDM/LibSFDM.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

//////////////////////////////////////////////////////////////////////////////

/// This class defines an action that creates a space in the mesh
/// with SFDM shape functions, and a configurable order of polynomial.
/// Default polynomial order = 0.
/// that returns information about the mesh
/// @author Willem Deconinck
class SFDM_API CreateSFDFields : public solver::Action
{
public: // typedefs

    typedef boost::shared_ptr<CreateSFDFields> Ptr;
    typedef boost::shared_ptr<CreateSFDFields const> ConstPtr;

public: // functions

  /// constructor
  CreateSFDFields( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "CreateSFDFields"; }

  virtual void execute();

}; // end CreateSFDFields


////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_CreateSFDFields_hpp
