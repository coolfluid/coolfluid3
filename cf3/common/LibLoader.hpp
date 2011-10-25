// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_LibLoader_hpp
#define cf3_common_LibLoader_hpp

#include "common/BoostFilesystem.hpp"

#include "common/CommonAPI.hpp"
#include "common/Exception.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

class Library;

////////////////////////////////////////////////////////////////////////////////

class Common_API LibLoadingError : public common::Exception {
public:

  /// Constructor
  LibLoadingError ( const common::CodeLocation& where, const std::string& what) :
    common::Exception(where, what, "LibLoadingError") {}

  /// Copy constructor
  LibLoadingError ( const LibLoadingError& e) throw () : Exception(e) {}

}; // LibLoadingError


////////////////////////////////////////////////////////////////////////////////

/// Class that explicit loads shared libraries
/// independently of the operating system
/// @author Tiago Quintino
class Common_API LibLoader : public boost::noncopyable {

public: // methods

  /// constructor
  LibLoader();

  /// virtual destructor
  virtual ~LibLoader();

  /// Loads a library and initiates it
  /// @throw LibLoadingError if loading fails for any reason
  void load_library(const std::string& lib);

  /// Unloads a library and initiates it
  /// @throw LibLoadingError if loading fails for any reason
  void unload_library( boost::shared_ptr<Library> lib );

  /// class interface to load a library depending on the operating system
  /// and the library loading algorithm
  /// @throw LibLoadingError if loading fails for any reason
  virtual void system_load_library(const std::string& lib) = 0;

  /// class interface to add paths to search for libraries
  ///
  virtual void set_search_paths(const std::vector< boost::filesystem::path >& paths) = 0;

  /// Gets the Class name
  static std::string type_name() { return "LibLoader"; }

}; // LibLoader

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_LibLoader_hpp
