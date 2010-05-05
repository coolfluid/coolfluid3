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

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Neu {

////////////////////////////////////////////////////////////////////////////////
    
/// Class defines the Neutral mesh format operations
/// @author Willem Deconinck
class NeuLib :
    public Common::ModuleRegister<NeuLib>
{
public:

  /// Static function that returns the module name.
  /// Must be implemented for the ModuleRegister template
  /// @return name of the module
  static std::string getModuleName() { return "Neu"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for the ModuleRegister template
  /// @return descripton of the module
  static std::string getModuleDescription()
  {
    return "This library implements the Neutral mesh format operations.";
  }

  /// Gets the Class name
  static std::string getClassName() { return "NeuLib"; }

}; // end NeuLib

////////////////////////////////////////////////////////////////////////////////

} // namespace Neu
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Neu_hpp
