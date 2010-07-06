#ifndef CF_Mesh_P1_hpp
#define CF_Mesh_P1_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"
#include "Common/ModuleRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro P1_API
/// @note build system defines P1_EXPORTS when compiling MeshTools files
#ifdef P1_EXPORTS
#   define P1_API      CF_EXPORT_API
#   define P1_TEMPLATE
#else
#   define P1_API      CF_IMPORT_API
#   define P1_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {

////////////////////////////////////////////////////////////////////////////////
    
  /// Class defines the initialization and termination of the library Mesh
  /// @author Tiago Quintino, Willem Deconinck
  class P1Lib :
      public Common::ModuleRegister<P1Lib>
  {
  public:

    /// Static function that returns the module name.
    /// Must be implemented for the ModuleRegister template
    /// @return name of the module
    static std::string getModuleName() { return "P1"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for the ModuleRegister template
    /// @return descripton of the module
    static std::string getModuleDescription()
    {
      return "This library implements the mesh manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "P1Lib"; }

  }; // end P1Lib

////////////////////////////////////////////////////////////////////////////////

} // namespace P1
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_hpp
