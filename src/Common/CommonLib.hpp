#ifndef CF_Common_CommonLib_hpp
#define CF_Common_CommonLib_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "Common/ModuleRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/// Class defines the initialization and termination of the library Mesh
/// @author Tiago Quintino
class CommonLib : public Common::ModuleRegister<CommonLib>
{
public:

  /// Static function that returns the module name.
  /// Must be implemented for the ModuleRegister template
  /// @return name of the module
  static std::string getModuleName() { return "Common"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for the ModuleRegister template
  /// @return descripton of the module
  static std::string getModuleDescription()
  {
    return "This library implements the Common API.";
  }

  /// Gets the Class name
  static std::string getClassName() { return "CommonLib"; }

  /// Start profiling
  virtual void initiate();

  /// Stop profiling
  virtual void terminate();

}; // CommonLib

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CommonLib_hpp
