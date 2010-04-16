#ifndef CF_MathAPI_HH
#define CF_MathAPI_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Math_API
/// @note build system defines Math_EXPORTS when compiling MathTools files
#ifdef Math_EXPORTS
#   define Math_API      CF_EXPORT_API
#   define Math_TEMPLATE
#else
#   define Math_API      CF_IMPORT_API
#   define Math_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  /// Basic Classes for Mathematical applications used by the CF
  namespace Math {}

} // namespace Math

////////////////////////////////////////////////////////////////////////////////

#endif // CF_MathAPI_HH
