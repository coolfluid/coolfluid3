// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Group_hpp
#define cf3_common_Group_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

/// @brief Component for grouping other components
///
/// Component class adding no extra functionality. Its purpose is to indicate
/// to the user this component only groups other components.
/// A GUI could use this information to give this component a "folder" icon.
/// @author Tiago Quintino
class Common_API Group : public Component
{

public: // functions

  /// Contructor
  /// @param name of the component
  Group ( const std::string& name );

  /// Virtual destructor
  virtual ~Group();

  /// Get the class name
  static std::string type_name () { return "Group"; }

}; // Group

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Group_hpp
