// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_QwtTab_LibQwtTab_hpp
#define CF_UI_QwtTab_LibQwtTab_hpp

//header
#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////

/// Define the macro QwtTab_API
/// @note build system defines COOLFLUID_UI_QWTTAB_EXPORTS when compiling QwtTabTools files
#ifdef COOLFLUID_UI_QWTTAB_EXPORTS
#   define QwtTab_API      CF_EXPORT_API
#   define QwtTab_TEMPLATE
#else
#   define QwtTab_API      CF_IMPORT_API
#   define QwtTab_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace QwtTab {

////////////////////////////////////////////////////////////////////////////

/// Class defines the initialization and termination of the library QwtTab
/// @author Tiago Quintino
class QwtTab_API LibQwtTab :
        public Common::CLibrary
{
public:

    typedef boost::shared_ptr<LibQwtTab> Ptr;
    typedef boost::shared_ptr<LibQwtTab const> ConstPtr;

    /// Constructor
    LibQwtTab ( const std::string& name) : Common::CLibrary(name) {   }

    void new_plot_signature( Common::SignalArgs & args );

public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.UI.QwtTab"; }

    /// Static function that returns the module name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "QwtTab"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
        return "This library implements the Qwt tab for the GUI.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibQwtTab"; }

protected:

    /// initiate library
    virtual void initiate_impl();

    /// terminate library
    virtual void terminate_impl();

private:

    int m_argc;

    int m_tabIndex;

}; // LibQwtTab

////////////////////////////////////////////////////////////////////////////

} // LibQwtTab
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_UI_QwtTab_LibQwtTab_hpp
