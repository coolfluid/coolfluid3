// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"
#include "Common/CRoot.hpp"
#include "Common/CGroup.hpp"

#include "RDM/Scalar/LibScalar.hpp"

namespace CF {
namespace RDM {

using namespace CF::Common;

CF::Common::RegistLibrary<LibScalar> LibScalar;

////////////////////////////////////////////////////////////////////////////////

void LibScalar::initiate_impl()
{
}

void LibScalar::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
