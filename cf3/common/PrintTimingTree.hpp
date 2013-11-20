// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_PrintTimingTree_hpp
#define cf3_common_PrintTimingTree_hpp

#include "common/Action.hpp"

#include "LibCommon.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////////////

/// Prints the timing tree for a root component
class Common_API PrintTimingTree : public Action
{
public: // functions

  /// Contructor
  /// @param name of the component
  PrintTimingTree ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "PrintTimingTree"; }

  virtual void execute();
private:
  // Root component to print timings from
  Handle<Component> m_root;
};

/////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_PrintTimingTree_hpp
