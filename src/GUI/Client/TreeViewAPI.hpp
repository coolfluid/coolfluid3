#ifndef CF_GUI_Client_TreeViewAPI_h
#define CF_GUI_Client_TreeViewAPI_h

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro TreeView_API
/// @note build system defines treeView_EXPORTS when compiling TreeView files
#ifdef treeView_EXPORTS
#   define TreeView_API CF_EXPORT_API
#else
#   define TreeView_API CF_IMPORT_API
#endif

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_TreeViewAPI_h
