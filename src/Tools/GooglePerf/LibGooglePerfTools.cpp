// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Tools/GooglePerf/LibGooglePerfTools.hpp"

#include "Common/CoreEnv.hpp"
#include "Common/DirPaths.hpp"
#include "Common/Log.hpp"
#include "Common/ObjectProvider.hpp"

namespace CF {
namespace Tools {
namespace GooglePerf {

  CF::Common::ForceLibRegist<LibGooglePerfTools> libGooglePerfTools;


LibGooglePerfTools::LibGooglePerfTools()
{

}

void LibGooglePerfTools::initiate() {

}

void LibGooglePerfTools::terminate() {

}

} // GooglePerf
} // Tools
} // CF
