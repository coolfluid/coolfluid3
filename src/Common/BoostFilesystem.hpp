// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_BoostAssert_hpp
#define CF_Common_BoostAssert_hpp

/// @deprecated upgrade the code to filesystem3 because filesystem2 is deprecated

// define the correct boost filesystem version
#define BOOST_FILESYSTEM_VERSION 2

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>

#endif // CF_Common_BoostAssert_hpp
