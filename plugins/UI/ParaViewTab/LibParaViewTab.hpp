// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_ParaViewTab_LibParaViewTab_hpp
#define CF_GUI_ParaViewTab_LibParaViewTab_hpp

////////////////////////////////////////////////////////////////////////////////

// header
#include "common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro ParaViewTab_API
/// @note build system defines COOLFLUID_UI_PARAVIEWTAB_EXPORTS when compiling ParaViewTabTools files
#ifdef COOLFLUID_UI_PARAVIEWTAB_EXPORTS
#   define ParaViewTab_API      CF_EXPORT_API
#   define ParaViewTab_TEMPLATE
#else
#   define ParaViewTab_API      CF_IMPORT_API
#   define ParaViewTab_TEMPLATE CF_TEMPLATE_EXTERN
#endif

class pqApplicationCore;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace UI {
/// Basic Classes for ParaViewTab applications used by CF
namespace ParaViewTab {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the initialization and termination of the library ParaViewTab
/// @author Tiago Quintino
class ParaViewTab_API LibParaViewTab :
        public Common::CLibrary
{
public:

    typedef boost::shared_ptr<LibParaViewTab> Ptr;
    typedef boost::shared_ptr<LibParaViewTab const> ConstPtr;

    /// Constructor
    LibParaViewTab ( const std::string& name) : Common::CLibrary(name), m_appCore(nullptr) {   }

public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.UI.ParaViewTab"; }

    /// Static function that returns the module name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "ParaViewTab"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
        return "This library implements the 3D visualisation plugin.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibParaViewTab"; }

protected:

    /// initiate library
    virtual void initiate_impl();

    /// terminate library
    virtual void terminate_impl();

private:

    pqApplicationCore * m_appCore;

    int m_argc;

    int m_tabIndex;

}; // end LibParaViewTab

////////////////////////////////////////////////////////////////////////////////

} // ParaViewTab
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_ParaViewTab_LibParaViewTab_hpp
