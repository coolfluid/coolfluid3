// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"

#include "Common/NoProfiling.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < NoProfiling, CodeProfiler, LibCommon > NoProfiling_Builder;

NoProfiling::NoProfiling(const std::string& name) : CodeProfiler(name)
{
}

NoProfiling::~NoProfiling()
{
}

/////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
