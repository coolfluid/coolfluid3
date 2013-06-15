// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_OSystem_hpp
#define cf3_common_OSystem_hpp

#include <boost/shared_ptr.hpp>

#include "common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

  class OSystemLayer;
  class LibLoader;

////////////////////////////////////////////////////////////////////////////////

/// Represents the operating system
/// @author Tiago Quintino
class Common_API OSystem : public boost::noncopyable {

public: // methods

  /// @return ProcessInfo object
  boost::shared_ptr<common::OSystemLayer> layer();

  /// @return LibLoader object
  boost::shared_ptr<common::LibLoader> lib_loader();

  /// @return the single object that represents the operating system
  static OSystem& instance();
  
  /// Set an environment variable
  static void setenv(const std::string& name, const std::string& value);

private: // functions

  /// constructor
  OSystem ();
  /// destructor
  ~OSystem ();

private: // data

  /// memory usage object
  boost::shared_ptr<common::OSystemLayer> m_layer;
  /// libloader object
  boost::shared_ptr<common::LibLoader> m_lib_loader;

}; // class OSystem

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_OSystem_hpp
