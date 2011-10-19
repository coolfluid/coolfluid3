// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "BlockMeshReader/LibBlockMeshReader.hpp"

namespace cf3 {
namespace BlockMeshReader {

cf3::common::RegistLibrary<LibBlockMeshReader> libBlockMeshReader;

////////////////////////////////////////////////////////////////////////////////

void LibBlockMeshReader::initiate_impl()
{
}

void LibBlockMeshReader::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // BlockMeshReader
} // cf3
