// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CGroupActions_hpp
#define CF_Common_CGroupActions_hpp

#include "Common/CAction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

class Common_API CGroupActions : public CAction {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CGroupActions> Ptr;
  typedef boost::shared_ptr<CGroupActions const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CGroupActions ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "CGroupActions"; }

  /// execute the action
  virtual void execute ();
  
};

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CGroupActions_hpp
