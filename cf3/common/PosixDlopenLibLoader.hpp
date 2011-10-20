// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_PosixDlopenLibLoader_hpp
#define cf3_common_PosixDlopenLibLoader_hpp

#include "common/LibLoader.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

class Common_API PosixDlopenLibLoader : public LibLoader {

public: // functions

  /// constructor
  PosixDlopenLibLoader();

  /// virtual destructor
  virtual ~PosixDlopenLibLoader();

  /// class interface to load a library depending on the operating system
  /// and the library loading algorithm
  /// @throw LibLoadingError if loading fails for any reason
  ///
  virtual void system_load_library(const std::string& lib);

  /// class interface to add paths to search for libraries
  ///
  virtual void set_search_paths(const std::vector< boost::filesystem::path >& paths);

  protected:

  void* call_dlopen(const boost::filesystem::path& fpath);

  private: // data

    /// paths where to search for the libraries to load
    std::vector< boost::filesystem::path > m_search_paths;

}; // PosixDlopenLibLoader

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_PosixDlopenLibLoader_hpp
