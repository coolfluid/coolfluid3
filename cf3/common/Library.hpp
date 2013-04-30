// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Library_hpp
#define cf3_common_Library_hpp

/////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

struct LibRegistInfo;

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component class for a library
/// @author Quentin Gasper
class Common_API Library : public Component {

public:

  /// Contructor
  /// @param name of Library
  Library( const std::string& name);

  /// Virtual destructor.
  virtual ~Library();

  /// Get the class name
  static std::string type_name() { return "Library"; }

  bool is_initiated() const { return m_is_initiated; }

  /// initiate library
  virtual void initiate();

  /// terminate library
  virtual void terminate();

  /// @returns a string with the kernel version which compiled this library
  std::string lib_kversion();

  /// @returns a string with this library version
  /// Equal to kernel version for liraries distributed with kernel
  virtual std::string lib_version();

protected:

  bool m_is_initiated;

}; // Library

/////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////


#endif // cf3_common_Library_hpp
