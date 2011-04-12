#ifndef LIBSERVERPARAVIEW_HPP
#define LIBSERVERPARAVIEW_HPP

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro ServerParaView_API
/// @note build system defines COOLFLUID_UI_ServerParaView_EXPORTS when compiling ParaViewTools files
#ifdef COOLFLUID_UI_PARAVIEW_EXPORTS
#   define ServerParaView_API      CF_EXPORT_API
#   define ServerParaView_TEMPLATE
#else
#   define ServerParaView_API      CF_IMPORT_API
#   define ServerParaView_TEMPLATE CF_TEMPLATE_EXTERN
#endif

///////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ServerParaView {

///////////////////////////////////////////////////////////////////////////////

class LibServerParaView :
    public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibServerParaView> Ptr;
  typedef boost::shared_ptr<LibServerParaView const> ConstPtr;

  /// Constructor
  LibServerParaView ( const std::string& name) : Common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.UI.ServerParaView"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "ServerParaView"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
      return "This library implements the Paraview Server API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibServerParaView"; }

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

#endif // LIBSERVERPARAVIEW_HPP
