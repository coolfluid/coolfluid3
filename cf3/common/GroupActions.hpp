// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_GroupActions_hpp
#define cf3_common_GroupActions_hpp

#include "common/Action.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////////////

/// @brief Action component that executes all contained actions.
///
/// Contained actions must be of a derived type Action, or of the type Link, which
/// points to a derived Action type.
///
/// @author Willem Deconinck
class Common_API GroupActions : public Action {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<GroupActions> Ptr;
  typedef boost::shared_ptr<GroupActions const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  GroupActions ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "GroupActions"; }

  /// execute the action
  virtual void execute ();

};

/////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_GroupActions_hpp
