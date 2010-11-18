// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_BlockMeshReader_LibBlockMeshReader_hpp
#define CF_BlockMeshReader_LibBlockMeshReader_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"
#include "Common/LibraryRegister.hpp"

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
    public Common::LibraryRegister<LibBlockMeshReader>
{
public:

  /// Static function that returns the module name.
  /// Must be implemented for the LibraryRegister template
  /// @return name of the module
  static std::string library_name() { return "BlockMeshReader"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for the LibraryRegister template
  /// @return descripton of the module
  static std::string library_description()
  {
    return "This library implements some BlockMeshReader compatibility functions.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibBlockMeshReader"; }

  /// Start profiling
  virtual void initiate();

  /// Stop profiling
  virtual void terminate();
}; // end LibBlockMeshReader

////////////////////////////////////////////////////////////////////////////////

} // namespace BlockMeshReader
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_BlockMeshReader_LibBlockMeshReader_hpp
