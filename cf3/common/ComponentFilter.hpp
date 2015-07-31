// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_ComponentFilter_hpp
#define cf3_common_ComponentFilter_hpp

#include "common/Component.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

///////////////////////////////////////////////////////////////////////////////////////

/// Component that provides a function call operator returning a boolean, useful as configurable filtering predicate
class Common_API ComponentFilter : public Component
{
public:
  /// Contructor
  /// @param name of the component
  ComponentFilter ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "ComponentFilter"; }

  /// Implement this to return true or false depending on the criteria of the filter
  virtual bool operator()(const Component& component) = 0;

  /// Handle version, calls the reference version
  /// Returns false for null components, override to change this behavior
  virtual bool operator()(const Handle<Component const>& component);
};

/////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_ComponentFilter_hpp
