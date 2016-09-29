// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_TaggedComponentFilter_hpp
#define cf3_common_TaggedComponentFilter_hpp

#include "common/Component.hpp"
#include "common/ComponentFilter.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

///////////////////////////////////////////////////////////////////////////////////////

/// ComponentFilter that takes a tag
class Common_API TaggedComponentFilter : public ComponentFilter
{
public:
  /// Contructor
  /// @param name of the component
  TaggedComponentFilter ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "TaggedComponentFilter"; }

  /// Implement this to return true or false depending on the criteria of the filter
  virtual bool operator()(const Component& component) const;

private:
  std::string m_tag;
};

/////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_TaggedComponentFilter_hpp
