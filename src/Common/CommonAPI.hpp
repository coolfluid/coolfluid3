#ifndef CF_Common_hh
#define CF_Common_hh

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  /// Common holds classes that abstract the Operating System and the Parallel environment
  /// and provide common facilities (like logging) to all other libraries.
  /// Common is the most basic of the Kernel libraries.
  namespace Common {

  /// Define the macro Common_API
  /// @note build system defines Common_EXPORTS when compiling Common files
  #ifdef Common_EXPORTS
  #   define Common_API CF_EXPORT_API
  #else
  #   define Common_API CF_IMPORT_API
  #endif


////////////////////////////////////////////////////////////////////////////////

  } // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_hh
