// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_BoostFilesystem_hpp
#define cf3_common_BoostFilesystem_hpp

/// @deprecated upgrade the code to filesystem3 because filesystem2 is deprecated

// define the correct boost filesystem version
#define BOOST_FILESYSTEM_VERSION 2

#include <boost/filesystem.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp>

#endif // cf3_common_BoostFilesystem_hpp
