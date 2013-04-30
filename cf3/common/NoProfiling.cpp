// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/LibCommon.hpp"

#include "common/NoProfiling.hpp"

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < NoProfiling, CodeProfiler, LibCommon > NoProfiling_Builder;

NoProfiling::NoProfiling(const std::string& name) : CodeProfiler(name)
{
}

NoProfiling::~NoProfiling()
{
}

/////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
