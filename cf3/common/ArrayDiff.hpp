// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_ArrayDiff_hpp
#define cf3_common_ArrayDiff_hpp

#include <boost/scoped_ptr.hpp>

#include "common/Component.hpp"
#include "common/Action.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
  
///////////////////////////////////////////////////////////////////////////////////////

// Determine if two arrays are different
class Common_API ArrayDiff : public Action
{
public:
  ArrayDiff(const std::string& name);
  virtual ~ArrayDiff();
  
  static std::string type_name () { return "ArrayDiff"; }
  
  virtual void execute();
};
  
///////////////////////////////////////////////////////////////////////////////////////
  
} // common
} // cf3

#endif // cf3_common_ArrayDiff_hpp
