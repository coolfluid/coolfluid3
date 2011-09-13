// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_BlockMeshReader_LibBlockMeshReader_hpp
#define CF_BlockMeshReader_LibBlockMeshReader_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro BlockMeshReader_API
/// @note build system defines COOLFLUID_BLOCKMESH_READER_EXPORTS when compiling
/// BlockMeshReader files
#ifdef COOLFLUID_BLOCKMESH_READER_EXPORTS
#   define BlockMeshReader_API      CF_EXPORT_API
#   define BlockMeshReader_TEMPLATE
#else
#   define BlockMeshReader_API      CF_IMPORT_API
#   define BlockMeshReader_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace BlockMeshReader {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the BlockMeshReadertral mesh format operations
/// @author Willem Deconinck
class BlockMeshReader_API LibBlockMeshReader :
    public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibBlockMeshReader> Ptr;
  typedef boost::shared_ptr<LibBlockMeshReader const> ConstPtr;

  /// Constructor
  LibBlockMeshReader ( const std::string& name) : Common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.BlockMeshReader"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "BlockMeshReader"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements some BlockMeshReader compatibility functions.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibBlockMeshReader"; }

protected:
  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibBlockMeshReader

////////////////////////////////////////////////////////////////////////////////

} // BlockMeshReader
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_BlockMeshReader_LibBlockMeshReader_hpp
