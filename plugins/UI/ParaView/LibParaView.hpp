#ifndef LIBParaView_HPP
#define LIBParaView_HPP

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro ParaView_API
/// @note build system defines COOLFLUID_UI_ParaView_EXPORTS when compiling ParaViewTools files
#ifdef COOLFLUID_UI_PARAVIEW_EXPORTS
#   define ParaView_API      CF_EXPORT_API
#   define ParaView_TEMPLATE
#else
#   define ParaView_API      CF_IMPORT_API
#   define ParaView_TEMPLATE CF_TEMPLATE_EXTERN
#endif

///////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ParaView {

///////////////////////////////////////////////////////////////////////////////

class LibParaView :
    public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibParaView> Ptr;
  typedef boost::shared_ptr<LibParaView const> ConstPtr;

  /// Constructor
  LibParaView ( const std::string& name) : Common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.UI.ParaView"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "ParaView"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
      return "This library implements the Paraview Server plugin.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibParaView"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();
};

////////////////////////////////////////////////////////////////////////////////

} // ParaView
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // LIBParaView_HPP
