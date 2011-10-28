// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "SFDM/LibSFDM.hpp"

namespace cf3 {
namespace SFDM {

cf3::common::RegistLibrary<LibSFDM> LibSFDM;

////////////////////////////////////////////////////////////////////////////////

void LibSFDM::initiate_impl()
{
}

void LibSFDM::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3
