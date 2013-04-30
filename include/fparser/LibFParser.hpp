// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef LibFParser_hpp
#define LibFParser_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CF.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro FParser_API
/// @note build system defines FParser_EXPORTS when compiling FParser files
#ifdef FParser_EXPORTS
#   define FParser_API      CF3_EXPORT_API
#   define FParser_TEMPLATE
#else
#   define FParser_API      CF3_IMPORT_API
#   define FParser_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

#endif // LibFParser_hpp
