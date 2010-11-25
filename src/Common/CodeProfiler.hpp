// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CodeProfiler_hpp
#define CF_Common_CodeProfiler_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/path.hpp>

#include "Common/Component.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  //////////////////////////////////////////////////////////////////////////////

  class Common_API CodeProfiler : public Component
  {
  public:

    /// constructor
    CodeProfiler(const std::string& name);

    /// virtual destructor
    virtual ~CodeProfiler();

    /// this class type
    static std::string type_name() { return "CodeProfiler"; }

    /// start profiling
    virtual void start_profiling() = 0;

    /// sttop profiling
    virtual void stop_profiling() = 0;

    /// define the path of the file where to dump profiling information
    virtual void set_file_path(const boost::filesystem::path & path) = 0;

    /// Configuration Options
    static void define_config_properties ( Common::PropertyList& options ) {}

  private: // helper functions

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

  }; // class CodeProfiler

  //////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CodeProfiler_hpp
