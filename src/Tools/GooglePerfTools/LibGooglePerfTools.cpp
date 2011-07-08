// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Tools/GooglePerfTools/LibGooglePerfTools.hpp"

#include "Common/Core.hpp"
#include "Common/RegistLibrary.hpp"

namespace CF {
namespace Tools {
namespace GooglePerfTools {

CF::Common::RegistLibrary<LibGooglePerfTools> libGooglePerfTools;

void LibGooglePerfTools::initiate_impl()
{
}

void LibGooglePerfTools::terminate_impl()
{
}

} // GooglePerfTools
} // Tools
} // CF
