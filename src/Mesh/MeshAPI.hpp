#ifndef CF_Mesh_HH
#define CF_Mesh_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Mesh_API
/// @note build system defines Mesh_EXPORTS when compiling MeshTools files
#ifdef Mesh_EXPORTS
#   define Mesh_API      CF_EXPORT_API
#   define Mesh_TEMPLATE
#else
#   define Mesh_API      CF_IMPORT_API
#   define Mesh_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  /// Basic Classes for Mesh applications used by CF
  namespace Mesh {
    
  } // namespace Mesh

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_HH
