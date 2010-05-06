#ifndef CF_GmshAPI_hpp
#define CF_GmshAPI_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Gmsh_API
/// @note build system defines Gmsh_EXPORTS when compiling Gmsh files
#ifdef Gmsh_EXPORTS
#   define Gmsh_API      CF_EXPORT_API
#   define Gmsh_TEMPLATE
#else
#   define Gmsh_API      CF_IMPORT_API
#   define Gmsh_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Gmsh {

////////////////////////////////////////////////////////////////////////////////
    
/// Class defines the Gmshtral mesh format operations
/// @author Willem Deconinck
class GmshLib :
    public Common::ModuleRegister<GmshLib>
{
public:

  /// Static function that returns the module name.
  /// Must be implemented for the ModuleRegister template
  /// @return name of the module
  static std::string getModuleName() { return "Gmsh"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for the ModuleRegister template
  /// @return descripton of the module
  static std::string getModuleDescription()
  {
    return "This library implements the Gmsh mesh format operations.";
  }

  /// Gets the Class name
  static std::string getClassName() { return "GmshLib"; }

}; // end GmshLib

////////////////////////////////////////////////////////////////////////////////

} // namespace Gmsh
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Gmsh_hpp
