#ifndef CF_CGNSAPI_hpp
#define CF_CGNSAPI_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ModuleRegister.hpp"
#include "Common/ExportAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro CGNS_API
/// @note build system defines CGNS_EXPORTS when compiling CGNS files
#ifdef CGNS_EXPORTS
#   define CGNS_API      CF_EXPORT_API
#   define CGNS_TEMPLATE
#else
#   define CGNS_API      CF_IMPORT_API
#   define CGNS_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace CGNS {

////////////////////////////////////////////////////////////////////////////////
    
/// Class defines the CGNS mesh format operations
/// @author Willem Deconinck
class CGNS_API CGNSLib :
    public CF::Common::ModuleRegister<CGNSLib>
{
public:

  /// Static function that returns the module name.
  /// Must be implemented for the ModuleRegister template
  /// @return name of the module
  static std::string getModuleName() { return "CGNS"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for the ModuleRegister template
  /// @return descripton of the module
  static std::string getModuleDescription()
  {
    return "This library implements the CGNS mesh format operations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "CGNSLib"; }

}; // end CGNSLib

////////////////////////////////////////////////////////////////////////////////

} // namespace CGNS
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_CGNS_hpp
