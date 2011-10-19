// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UI_QwtTab_LibQwtTab_hpp
#define cf3_UI_QwtTab_LibQwtTab_hpp

//header
#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////

/// Define the macro QwtTab_API
/// @note build system defines COOLFLUID_UI_QWTTAB_EXPORTS when compiling QwtTabTools files
#ifdef COOLFLUID_UI_QWTTAB_EXPORTS
#   define QwtTab_API      CF3_EXPORT_API
#   define QwtTab_TEMPLATE
#else
#   define QwtTab_API      CF3_IMPORT_API
#   define QwtTab_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace QwtTab {

////////////////////////////////////////////////////////////////////////////

/// Class defines the initialization and termination of the library QwtTab
/// @author Tiago Quintino
class QwtTab_API LibQwtTab :
        public common::CLibrary
{
public:

    typedef boost::shared_ptr<LibQwtTab> Ptr;
    typedef boost::shared_ptr<LibQwtTab const> ConstPtr;

    /// Constructor
    LibQwtTab ( const std::string& name) : common::CLibrary(name) {   }

    void new_plot_signature( common::SignalArgs & args );

public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.UI.QwtTab"; }

    /// Static function that returns the library name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "QwtTab"; }

    /// Static function that returns the description of the library.
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
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // CF3_UI_QwtTab_LibQwtTab_hpp
