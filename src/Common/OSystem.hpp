// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_OSystem_hpp
#define CF_Common_OSystem_hpp

#include <boost/shared_ptr.hpp>

#include "Common/CF.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  class OSystemLayer;
  class LibLoader;

////////////////////////////////////////////////////////////////////////////////

/// Represents the operating system
/// @author Tiago Quintino
class Common_API OSystem : public boost::noncopyable {

public: // methods

  /// @return ProcessInfo object
  boost::shared_ptr<Common::OSystemLayer> layer();

  /// @return LibLoader object
  boost::shared_ptr<Common::LibLoader> lib_loader();

  /// @return the single object that represents the operating system
  static OSystem& instance();

private: // functions

  /// constructor
  OSystem ();
  /// destructor
  ~OSystem ();

private: // data

  /// memory usage object
  boost::shared_ptr<Common::OSystemLayer> m_layer;
  /// libloader object
  boost::shared_ptr<Common::LibLoader> m_lib_loader;

}; // class OSystem

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OSystem_hpp
