// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"
#include "common/Group.hpp"

#include "RDM/Schemes/LibSchemes.hpp"

namespace cf3 {
namespace RDM {

using namespace cf3::common;

cf3::common::RegistLibrary<LibSchemes> LibSchemes;

////////////////////////////////////////////////////////////////////////////////

void LibSchemes::initiate_impl()
{
}

void LibSchemes::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
