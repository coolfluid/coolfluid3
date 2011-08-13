// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_LibCommon_hpp
#define CF_Common_LibCommon_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CommonAPI.hpp"
#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// COOLFluiD Classes
namespace CF {

/// Common Classes for Component Environment
namespace Common {

/// Class defines the initialization and termination of the library Mesh
/// @author Tiago Quintino
class Common_API LibCommon : public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibCommon> Ptr;
  typedef boost::shared_ptr<LibCommon const> ConstPtr;

  /// Constructor
  LibCommon ( const std::string& name) : Common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Common"; }

  /// Static function that returns the library name.
  /// Must be implemented for CLibrary rCFegistration
  /// @return name of the library
  static std::string library_name() { return "Common"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library
  static std::string library_description()
  {
    return "This library implements the Common API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibCommon"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // LibCommon

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#include "Common/Tags.hpp"

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_LibCommon_hpp
