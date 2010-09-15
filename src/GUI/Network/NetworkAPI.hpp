// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Network_NetworkAPI_h
#define CF_GUI_Network_NetworkAPI_h

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Network_API
/// @note build system defines network_EXPORTS when compiling Network files
#ifdef Network_EXPORTS
#   define Network_API CF_EXPORT_API
#else
#   define Network_API CF_IMPORT_API
#endif

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Network_NetworkAPI_h
