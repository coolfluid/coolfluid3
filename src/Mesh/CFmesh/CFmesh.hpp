#ifndef CF_CFmesh_HH
#define CF_CFmesh_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro CFmesh_API
/// @note build system defines CFmesh_EXPORTS when compiling CFmesh files
#ifdef CFmesh_EXPORTS
#   define CFmesh_API      CF_EXPORT_API
#   define CFmesh_TEMPLATE
#else
#   define CFmesh_API      CF_IMPORT_API
#   define CFmesh_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Mesh {
    
    /// Basic Classes for using the CFmesh mesh format
    namespace CFmesh {
      
    } // namespace CFmesh
    
  } // namespace Mesh

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_CFmesh_HH
