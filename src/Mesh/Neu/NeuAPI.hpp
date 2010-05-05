#ifndef CF_NeuAPI_hpp
#define CF_NeuAPI_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Neu_API
/// @note build system defines Neu_EXPORTS when compiling Neu files
#ifdef Neu_EXPORTS
#   define Neu_API      CF_EXPORT_API
#   define Neu_TEMPLATE
#else
#   define Neu_API      CF_IMPORT_API
#   define Neu_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Mesh {
    
    /// Basic Classes for using the Neu mesh format
    namespace Neu {
      
    } // namespace Neu
    
  } // namespace Mesh

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Neu_hpp
