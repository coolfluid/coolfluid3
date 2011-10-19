// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_CGroup_hpp
#define cf3_common_CGroup_hpp

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
class Common_API CGroup : public Component {

public: // typedefs

  /// pointer to this type
  typedef boost::shared_ptr<CGroup> Ptr;
  typedef boost::shared_ptr<CGroup const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CGroup ( const std::string& name );

  /// Virtual destructor
  virtual ~CGroup();

  /// Get the class name
  static std::string type_name () { return "CGroup"; }

}; // CGroup

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_CGroup_hpp
