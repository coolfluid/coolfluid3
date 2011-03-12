// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CLibrary_hpp
#define CF_Common_CLibrary_hpp

/////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

struct LibRegistInfo;

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component class for a library
/// @author Quentin Gasper
class Common_API CLibrary : public Component {

public:

  typedef boost::shared_ptr<CLibrary> Ptr;
  typedef boost::shared_ptr<CLibrary const> ConstPtr;

  /// Contructor
  /// @param name of Library
  CLibrary( const std::string& name);

  /// Virtual destructor.
  virtual ~CLibrary();

  /// Get the class name
  static std::string type_name() { return "CLibrary"; }

  /// initiate library
  virtual void initiate() = 0;

  /// terminate library
  virtual void terminate() = 0;

  /// @returns a string with the kernel version which compiled this library
  std::string lib_kversion();

  /// @returns a string with this library version
  /// Equal to kernel version for liraries distributed with kernel
  virtual std::string lib_version();

}; // CLibrary

/////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////


#endif // CF_Common_CLibrary_hpp
