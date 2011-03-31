// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_ParaView_LibParaView_hpp
#define CF_GUI_ParaView_LibParaView_hpp

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

class pqApplicationCore;

////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace UI {
/// Basic Classes for ParaView applications used by CF
namespace ParaView {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the initialization and termination of the library ParaView
/// @author Tiago Quintino
class ParaView_API LibParaView :
        public Common::CLibrary
{
public:

    typedef boost::shared_ptr<LibParaView> Ptr;
    typedef boost::shared_ptr<LibParaView const> ConstPtr;

    /// Constructor
    LibParaView ( const std::string& name) : Common::CLibrary(name), m_appCore(nullptr) {   }

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
        return "This library implements the Graphical Interface API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibParaView"; }

protected:

    /// initiate library
    virtual void initiate_impl();

    /// terminate library
    virtual void terminate_impl();

private:

    pqApplicationCore * m_appCore;

    int m_argc;

    int m_tabIndex;

}; // end LibParaView

////////////////////////////////////////////////////////////////////////////////

} // ParaView
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_ParaView_LibParaView_hpp
